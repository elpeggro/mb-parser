#!/usr/bin/python3
import sys
import os
import argparse
import math

# Total number of macroblocks in a single frame for the given vertical resolution. Calculated with 16:9 resolution and
# 16x16 px macroblocks.
MACROBLOCKS = {144: 144, 240: 400, 360: 900, 480: 1602, 720: 3600, 1080: 8100, 1440: 14400, 2160: 32400}

def get_graph_from_file(path: str):
    """Reads the file and returns the frame numbers in bytestream order, the graph, and the largest reference.

    The graph has the structure of a dictionary of the form:
        frame -> [(referenced by frame 1, number of macroblock references), (referenced by frame 2, ...), ...]
    """
    bytestream_frame_order = list()
    graph = dict()
    max_weight = 0
    edge_count = 0
    sink_count = 0
    with open(path, 'r') as f:
        # Consume header line
        f.readline()
        for line in f:
            line_split = line.split()
            if len(line_split) != 3:
                raise ValueError('Invalid line: ' + line)
            referenced_frame = int(line_split[0])
            edge_weight = int(line_split[1])
            referenced_by_frame = int(line_split[2])
            if len(bytestream_frame_order) == 0 or bytestream_frame_order[-1] != referenced_frame:
                bytestream_frame_order.append(referenced_frame)
            if edge_weight == 0:
                # Sink
                sink_count += 1
                continue
            if referenced_frame not in graph:
                graph[referenced_frame] = [(referenced_by_frame, edge_weight)]
            else:
                graph[referenced_frame].append((referenced_by_frame, edge_weight))
            if edge_weight > max_weight:
                max_weight = edge_weight
            edge_count += 1
    print('Vertices: ' + str(len(bytestream_frame_order)) + ' Edges: ' + str(edge_count) + ' Sinks: ' + str(sink_count))
    return bytestream_frame_order, graph, max_weight


def walk(frame: int, graph: dict, max_weight_influence: int, mb_threshold: int, chain_length: int):
    max_chain_length = chain_length + 1
    if frame not in graph:
        # Sink
        return 0, max_chain_length
    references = graph[frame]
    weight_acc = 0
    for referenced_by_frame, weight in references:
        new_weight_influence = min(max_weight_influence, weight)
        # We count even small references but break out of the recursion. Ignoring small references completely would
        # treat frames that have only small references equally with frames without references, which is not what we
        # want.
        weight_acc += new_weight_influence
        if new_weight_influence < mb_threshold:
            continue
        rec_weight, rec_chain_length = walk(referenced_by_frame, graph, new_weight_influence, mb_threshold,
                chain_length + 1)
        weight_acc += rec_weight
        max_chain_length = max(max_chain_length, rec_chain_length)
    return weight_acc, max_chain_length


def calculate_weights(bytestream_frame_order: list, graph: dict, mb_threshold: int):
    ret = list()
    max_chain_length = 0
    for frame in bytestream_frame_order:
        weight, chain_length = walk(frame, graph, math.inf, mb_threshold, 0)
        max_chain_length = max(max_chain_length, chain_length)
        ret.append((frame, weight))
    print('Maximum chain length: ' + str(max_chain_length))
    return ret


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('input', help='input file')
    parser.add_argument('output', help='output file')
    parser.add_argument('threshold', type=float, help='specifies the minimum amount (in percent of total number of\
            macroblocks in a single frame) of macroblocks required for a reference to be used as a chain link')
    modegroup = parser.add_mutually_exclusive_group(required=True)
    modegroup.add_argument('-r', '--relative', action='store_true', help='if specified, threshold is interpreted\
            relative to the "largest" (i.e., most macroblocks) reference in the segment')
    modegroup.add_argument('-a', '--absolute', type=int, help='vertical resolution (e.g., 144, 1080) of the video used\
            to generate the input file')
    args = parser.parse_args()
    print('Input: ' + str(os.path.basename(args.input)) + ' Output: ' + str(os.path.basename(args.output)))
    bytestream_frame_order, graph, max_weight = get_graph_from_file(args.input)
    if args.relative:
        ref_size = max_weight
        print('Max weight: ' + str(max_weight))
    else:
        ref_size = MACROBLOCKS[args.absolute]
    mb_threshold = math.ceil(ref_size * args.threshold / 100)
    print('Using threshold: ' + str(mb_threshold))
    weighted_list = calculate_weights(bytestream_frame_order, graph, mb_threshold)
    with open(args.output, 'w') as f:
        for frame, weight in weighted_list:
            f.write(str(frame) + ' ' + str(weight) + '\n')

