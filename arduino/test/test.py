#!/bin/env python3

import sys
import argparse
import mido


parser = argparse.ArgumentParser(
                    prog = 'test',
                    description = 'What the program does',
                    epilog = 'Text at the bottom of help')

parser.add_argument('-c', '--channel', type=int)      # option that takes a value
parser.add_argument('-t', '--type', type=int)  # on/off flag
parser.add_argument('-a', '--address', type=int)  # on/off flag
parser.add_argument('-u', '--curve', type=int)  # on/off flag
parser.add_argument('-b', '--blink', action='append')  # on/off flag
parser.add_argument('-p', '--pulse', action='append')  # on/off flag
parser.add_argument('-0', '--off', action='append')  # on/off flag
parser.add_argument('-1', '--on', action='append')  # on/off flag




args = parser.parse_args()

outport = mido.open_output()

# note

if (args.type != None):
    outport.send(mido.Message( 'note_on', channel=0, note=0, velocity=args.type))

if (args.channel != None):
    outport.send(mido.Message( 'note_on', channel=0, note=1, velocity=args.channel-1))

if (args.address != None):
    outport.send(mido.Message( 'note_on', channel=0, note=2, velocity=args.address))

if (args.curve != None):
    outport.send(mido.Message( 'note_on', channel=0, note=3, velocity=args.curve))

if (args.blink != None):
    for led in args.blink:
        outport.send(mido.Message('note_on', channel=14, note=12 + int(led), velocity=1))

if (args.pulse != None):
    for led in args.pulse:
        outport.send(mido.Message('note_on', channel=14, note=12 + int(led), velocity=4))

if (args.on != None):
    for led in args.on:
        outport.send(mido.Message('note_on', channel=14, note=12 + int(led), velocity=5))

if (args.off != None):
    for led in args.off:
        outport.send(mido.Message('note_on', channel=14, note=12 + int(led), velocity=3))
# sysex
#outport.send(mido.Message('sysex', data=[0x7D,0x00,0x00,0x0]))

# midi clock
#outport.send(mido.Message.from_bytes([0xF8]))
