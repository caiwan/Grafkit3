import sys
import datetime
import argparse

import re

def sanitize_for_cpp(name):
    sanitized_name = re.sub(r'[^a-zA-Z0-9_]', '_', name)
    if re.match(r'^\d', sanitized_name):
        sanitized_name = '_' + sanitized_name
    return sanitized_name


def hexdump_to_header(input_file, array_name, create_header):            
    header_content = f"""/*
 * File: {input_file}
 * Generated on: {datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")}
 * Generated from a file. Do not edit.
 */
"""

    if create_header:
        header_content += f"""
#ifndef _{array_name.upper()}_H_
#define _{array_name.upper()}_H_

#ifdef __cplusplus
extern "C" {{
#endif 

#include <stdint.h>"

extern const uint8_t {array_name}[];"
extern const size_t {array_name}_len;"

#ifdef __cplusplus
}}

#endif //__cplusplus

#endif // _{array_name.upper()}_H_
"""
        
        print(header_content)
        
        pass

    else: 
        try:
            with open(input_file, "rb") as file:
                data = file.read()
    
            array_content = f"const uint8_t {array_name}[] = {{\n"
            array_len = f"const size_t {array_name}_len = {len(data)};\n"
            for i in range(0, len(data), 16):
                chunk = data[i:i+16]
                hex_bytes = ", ".join(f"0x{byte:02x}" for byte in chunk)
                array_content += f"    /* {i:08x} */   {hex_bytes},\n"
            array_content += "};\n"

            print(header_content + array_content + array_len)
    

        except FileNotFoundError:
            print(f"Error: File \"{input_file}\" not found.")
            sys.exit(1)
        except PermissionError:
            print(f"Error: Permission denied when accessing \"{input_file}\".")
            sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="Generate a C header file from a binary file.")
    parser.add_argument("--input_file", "-i", help="The binary file to process")
    parser.add_argument("--array_name", "-n", help="The name of the array to use in the header file")
    parser.add_argument("--header", "-x", help="Create a C header file", action="store_true")
    args = parser.parse_args()
    
    hexdump_to_header(args.input_file, sanitize_for_cpp(args.array_name), args.header)

if __name__ == "__main__":
    main()
