import argparse

import re


def sanitize_for_cpp(name):
    sanitized_name = re.sub(r"[^a-zA-Z0-9_]", "_", name)
    if re.match(r"^\d", sanitized_name):
        sanitized_name = "_" + sanitized_name
    return sanitized_name


def main():
    parser = argparse.ArgumentParser(description="Generate a C header file from a binary file.")
    parser.add_argument("--input_file", "-i", help="The binary file to process", required=True)

    parser.add_argument(
        "--array_name",
        "-n",
        help="The name of the array to use in the header file",
        required=True,
    )

    parser.add_argument(
        "--header",
        "-x",
        help="Create a C header file",
        action="store_true",
    )
    parser.add_argument("--output_file", "-o", help="The output file to write to", default=None)
    parser.add_argument("--template", "-t", help="The template file to use", default=None)

    parser.parse_args()

    # hexdump_to_header(args.input_file, sanitize_for_cpp(args.array_name), args.header)


if __name__ == "__main__":
    main()
