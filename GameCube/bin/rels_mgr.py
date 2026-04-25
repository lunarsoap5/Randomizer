#!/usr/bin/python3

"""
Injects arbitrary data into a GCI file
"""

import argparse
from dataclasses import dataclass
from io import BufferedReader
import sys
import os
import struct
from datetime import datetime, timezone
import logging
from enum import IntEnum
from typing import List

__author__ = "kipcode66"
__copyright__ = "Copyright 2024"
__credits__ = "kipcode66"
__license__ = "GPL3"
__version__ = "0.1.0"


MAX_REL_ENTRIES = 37
MAIN_REL_OFFSET = 0x2240

class RELSEntry:
    def __init__(self, id: int, size: int, offset: int, data: bytes):
        self.id = id
        self.size = size
        self.offset = offset
        self.data = data

    def __str__(self):
        offset = self.offset - 0x40 if self.offset != 0 else 0
        return f"ID: 0x{self.id:x} Size: 0x{self.size:x} Offset: 0x{offset:x}"

    def __repr__(self) -> str:
        return self.__str__()

class RELS:
    header: bytes
    mainRel: RELSEntry
    entries: List[RELSEntry]

    def __init__(self, header: bytes, mainRelData: bytes, entries: List[RELSEntry]):
        self.header = header
        self.mainRel = RELSEntry(0, len(mainRelData), 0x2240, mainRelData)
        self.entries = entries

    def __str__(self):
        return f"Main REL: {self.mainRel} Entries: {self.entries}"

    def __repr__(self) -> str:
        return self.__str__()

    def unpack(data: bytes) -> "RELS":
        entries = []
        for i in range(0, MAX_REL_ENTRIES):
            offset = 0x2084 + i * 0xC
            currentId = struct.unpack_from(">I", data, offset)[0]
            relSize = struct.unpack_from(">I", data, offset + 4)[0]
            relOffset = struct.unpack_from(">I", data, offset + 8)[0]
            relOffset += 0 if relOffset == 0 else 0x40
            isEmpty = currentId == 0 or relSize == 0 or relOffset == 0
            entries.append(RELSEntry(currentId, relSize, relOffset, data[relOffset:relOffset + relSize] if not isEmpty else b""))
        mainRelSize = struct.unpack_from(">I", data, 0x2080)[0]
        return RELS(data[:0x2080], data[0x2240:][:mainRelSize], entries)

    def pack(self) -> bytes:
        output = bytearray(self.header)
        output += b"\x00" * (4 + MAX_REL_ENTRIES * 0xC)
        struct.pack_into(">I", output, 0x2080, self.mainRel.size)
        for i, entry in enumerate(self.entries):
            offset = 0x2084 + i * 0xC
            struct.pack_into(">I", output, offset, entry.id)
            struct.pack_into(">I", output, offset + 4, entry.size)
            struct.pack_into(">I", output, offset + 8, entry.offset - (0x40 if entry.offset != 0 else 0))
        output += self.mainRel.data
        return output

def alignAddress(address: int, bits: int) -> int:
    return (address + (1 << bits) - 1) & ~((1 << bits) - 1)

def main():
    @dataclass
    class ParserArguments():
        command: str
        gci: BufferedReader
        region: str
        data: BufferedReader
        output: str
        id: int
        force: bool
        replace: bool

    # Parsing Functions
    def parseId(value: str) -> int:
        val = int(value, 0)
        if val < 1 or val > 0xFFFFFFFF:
            raise argparse.ArgumentTypeError(f"ID must be between 1 and 0xFFFFFFFF")
        return val

    parser = argparse.ArgumentParser(sys.argv[0], description="Packs data into a GCI file")
    parser.add_argument("-v", "--verbose", action="count", default=0, help="Increase verbosity")
    parser.add_argument("-V", "--version", action="version", version=f"%(prog)s {__version__}")
    subparsers = parser.add_subparsers(dest="command", metavar="command", help="Available commands are: list, add, extract, rm")
    list_parser = subparsers.add_parser("list", help="List all data in the GCI file")
    list_parser.add_argument("gci", type=argparse.FileType("rb"), help="GCI file to list data from")
    add_parser = subparsers.add_parser("add", help="Add data to the GCI file")
    add_parser.add_argument("gci", type=argparse.FileType("r+b"), help="GCI file to inject data into")
    add_parser.add_argument("id", type=parseId, help="ID to use for the data (default: 0x53454544 (SEED))")
    add_parser.add_argument("data", type=argparse.FileType("rb"), help="Provided to add to the GCI")
    add_parser.add_argument("-o", "--output", type=str, help="Optional output file. If not provided, the GCI file will be modified in place")
    add_parser.add_argument("-f", "--force", action="store_true", help="Force overwrite of existing GCI file")
    add_parser.add_argument("-r", "--replace", action="store_true", help="Replace existing data with the same ID")
    rm_parser = subparsers.add_parser("rm", help="Delete data from the GCI file")
    rm_parser.add_argument("gci", type=argparse.FileType("r+b"), help="GCI file to delete data from")
    rm_parser.add_argument("id", type=parseId, help="ID of the data to delete")
    rm_parser.add_argument("-o", "--output", type=str, help="Optional output file. If not provided, the GCI file will be modified in place")
    rm_parser.add_argument("-f", "--force", action="store_true", help="Force overwrite of existing GCI file")
    extract_parser = subparsers.add_parser("extract", help="Extract data from the GCI file")
    extract_parser.add_argument("gci", type=argparse.FileType("rb"), help="GCI file to extract data from")
    extract_parser.add_argument("id", type=parseId, help="ID of the data to extract")
    extract_parser.add_argument("output", type=argparse.FileType("wb"), help="Output file to write the extracted data to")
    args: ParserArguments = parser.parse_args()

    if not "command" in args or args.command is None:
        parser.print_help()
        sys.exit(0)

    # Set logging level
    logging_format = "[%(levelname)s]\t[%(asctime)s]\t%(pathname)s:%(lineno)d %(funcName)s: %(message)s"
    if args.verbose == 1:
        logging.basicConfig(level=logging.INFO, format=logging_format)
    elif args.verbose > 1:
        logging.basicConfig(level=logging.DEBUG, format=logging_format)

    if args.command in ["add", "rm"]:
        # Check if output file is not provided or input file is the same
        if (not args.output or args.output == args.gci.name) and not args.force:
            logging.error("Output file is not provided or the same as input file. Use -f to force overwrite")
            sys.exit(1)

    output = bytearray()

    # Read the GCI file
    gciBytes = args.gci.read()
    logging.debug(f"Read 0x{len(gciBytes)} bytes from GCI file")
    output.extend(gciBytes[:0x2240])

    # Assert the file is a REL mod
    logging.debug(f"internal filename: {gciBytes[0x8:0x28].decode('utf-8')}")
    if gciBytes[0x8:0x17].decode("utf-8") != "Custom REL File":
        logging.error("The provided GCI file is not a REL mod")
        sys.exit(1)

    # Read the RELS header
    rels: RELS = RELS.unpack(gciBytes)
    logging.debug(f"RELS : {rels}")

    if args.command == "add":
        # To add data
        # Check if data with the same ID exists
        for entry in rels.entries:
            if entry.id == args.id:
                if args.replace:
                    logging.debug(f"Found existing data with ID 0x{args.id:x}. Replacing")
                    break
                logging.error(f"Data with ID 0x{args.id:x} already exists. Use -r to replace")
                sys.exit(1)
        else:
            logging.debug(f"Data with ID 0x{args.id:x} not found")

        logging.debug(f"empty entries: {[(i, entry) for i, entry in enumerate(rels.entries) if entry.id == 0 or entry.size == 0]}")
        index, entry = next(((i, entry) for i, entry in enumerate(rels.entries) if entry.id == 0 or entry.size == 0), (None, None))

        if entry is None:
            logging.error("No space for new data")
            sys.exit(1)

        # Add entry
        rels.entries[index].id = args.id
        rels.entries[index].data = args.data.read()
        rels.entries[index].size = len(rels.entries[index].data)

        # Update the offsets
        offset = alignAddress(0x2240 + rels.mainRel.size, 2)
        for i, e in enumerate(rels.entries):
            if e.id == 0 or e.size == 0:
                continue
            rels.entries[i].offset = offset
            offset = alignAddress(offset + e.size, 2)

        # Resize the GCI file
        newSize = alignAddress(offset - 0x40, 13) + 0x40
        logging.debug(f"New size: 0x{newSize:x}")
        if len(output) < newSize:
            output += b"\x00" * (newSize - len(output))
        elif len(output) > newSize:
            output = output[:newSize]
        logging.debug(f"output size: 0x{len(output):x}")

        # Write the new RELS data
        header = rels.header
        output[:0x2080] = header
        output[0x2240:0x2240 + len(rels.mainRel.data)] = rels.mainRel.data
        logging.debug(f"mainRel: {output[0x2240:][:rels.mainRel.size]}")
        for i, e in enumerate(rels.entries):
            offset = 0x2084 + i * 0xC
            struct.pack_into(">I", output, offset, e.id)
            struct.pack_into(">I", output, offset + 4, e.size)
            struct.pack_into(">I", output, offset + 8, e.offset - (0x40 if e.offset != 0 else 0))
            output[e.offset:e.offset + e.size] = e.data

        # Update the Block count
        blocks = newSize // 0x2000
        struct.pack_into(">H", output, 0x38, blocks)

        # Update modified time
        totalSeconds = int((datetime.now(timezone.utc).replace(tzinfo=None) - datetime(2000, 1, 1)).total_seconds())
        struct.pack_into(">I", output, 0x28, totalSeconds)
    elif args.command == "list":
        print(f"Main REL size: 0x{rels.mainRel.size:x}")
        for i, entry in enumerate(rels.entries):
            if entry.id != 0:
                print(f"entry #{i}; {entry}")
        return
    elif args.command == "extract":
        for entry in rels.entries:
            if entry.id == args.id:
                args.output.write(entry.data)
                return
        logging.error(f"Data with ID 0x{args.id:x} not found")
        sys.exit(1)
    elif args.command == "rm":
        # To delete data
        for i, entry in enumerate(rels.entries):
            if entry.id == args.id:
                logging.debug(f"Found data to delete at entry #{i}")

                # Clear the entry
                rels.entries.pop(i)
                rels.entries.append(RELSEntry(0, 0, 0, b""))

                # Update the offsets
                offset = alignAddress(0x2240 + rels.mainRel.size, 2)
                for j, e in enumerate(rels.entries):
                    if e.id == 0 or e.size == 0:
                        continue
                    rels.entries[j].offset = offset
                    offset = alignAddress(offset + e.size, 2)

                # Resize the GCI file
                newSize = alignAddress(offset - 0x40, 13) + 0x40
                logging.debug(f"New size: 0x{newSize:x}")
                if len(output) < newSize:
                    output += b"\x00" * (newSize - len(output))
                elif len(output) > newSize:
                    output = output[:newSize]
                logging.debug(f"output size: 0x{len(output):x}")

                # Write the new RELS data
                header = rels.header
                output[:0x2080] = header
                output[0x2240:0x2240 + len(rels.mainRel.data)] = rels.mainRel.data
                for i, e in enumerate(rels.entries):
                    offset = 0x2084 + i * 0xC
                    struct.pack_into(">I", output, offset, e.id)
                    struct.pack_into(">I", output, offset + 4, e.size)
                    struct.pack_into(">I", output, offset + 8, e.offset - (0x40 if e.offset != 0 else 0))
                    output[e.offset:e.offset + e.size] = e.data

                # Update the Block count
                blocks = newSize // 0x2000
                struct.pack_into(">H", output, 0x38, blocks)

                # Update modified time
                totalSeconds = int((datetime.now(timezone.utc).replace(tzinfo=None) - datetime(2000, 1, 1)).total_seconds())
                struct.pack_into(">I", output, 0x28, totalSeconds)

                break
        else:
            logging.error(f"Data with ID 0x{args.id:x} not found")
            sys.exit(1)

    if args.output:
        with open(args.output, "wb") as file:
            file.write(output)
    else:
        args.gci.seek(0)
        args.gci.truncate(0)
        args.gci.write(output)

    logging.info("Done")

if __name__ == "__main__":
    main()
