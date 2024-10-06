import argparse
import datetime
import logging
from grafkit_tools import utils
import jinja2
import sys


logger = logging.getLogger(__name__)

# Jinja2 templates
TEMPLATE = """/*
 * File: {{ input_file }}
 * Generated on: {{ timestamp }}
 * Generated from a file. Do not edit.
 */
{% if create_header %}
#ifndef _{{ array_name.upper() }}_H_
#define _{{ array_name.upper() }}_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

extern const uint8_t {{ array_name }}[];
extern const size_t {{ array_name }}_len;

#ifdef __cplusplus
}

#endif // __cplusplus

#endif // _{{ array_name.upper() }}_H_
{% endif %}

{% if not create_header %}
#include <stdint.h>
#include <stddef.h>
const uint8_t {{ array_name }}[] = {
{% for chunk in data | generate_chunks -%}
    /* {{ "%08x" % chunk.offset }} */ {{ chunk.bytes | join(", ") }},
{% endfor %}
};
const size_t {{ array_name }}_len = {{ data|length }};
{% endif %}
"""


def hexdump_to_header(input_file, array_name, create_header, out_file=None, template_file=None):
    env = jinja2.Environment()
    utils.setup_env(env)

    with open(input_file, "rb") as file:
        data = bytearray(file.read())

    context = {
        "input_file": input_file,
        "timestamp": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "array_name": array_name,
        "create_header": create_header,
        "data": data,
    }

    if template_file:
        with open(template_file, "r") as file:
            template = file.read()
    else:
        template = TEMPLATE

    content = env.from_string(template).render(context)

    if out_file:
        with open(out_file, "w") as file:
            file.write(content)
    else:
        print(content)


def build_parser():
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
    return parser


def main():
    parser = build_parser()
    args = parser.parse_args()

    try:
        hexdump_to_header(
            args.input_file,
            utils.sanitize_string_for_cpp(args.array_name),
            args.header,
            args.output_file,
            args.template,
        )
    except Exception as e:
        logger.error(e, exc_info=True)
        sys.exit(1)


if __name__ == "__main__":
    main()
