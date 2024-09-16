import os
import datetime
import sys
import argparse
import logging

import yaml
from dataclasses import dataclass, asdict
from typing import List, Optional, Union
from marshmallow_dataclass import class_schema
from marshmallow import ValidationError

import jinja2
from grafkit_tools import utils

logger = logging.getLogger(__name__)


def build_parser():
    parser = argparse.ArgumentParser(description="Grafkit template generator")
    parser.add_argument(
        "--input-files",
        "-i",
        help="The data file to process",
        required=True,
        type=str,
        nargs="+",
        dest="input_files",
    )

    parser.add_argument(
        "--template-file",
        "-t",
        help="The template file to use",
        required=True,
        type=str,
        dest="template_file",
    )

    parser.add_argument(
        "--extra-includes",
        help="Extra include files",
        type=str,
        nargs="+",
        default=[],
        dest="extra_includes",
    )

    parser.add_argument(
        "--output",
        "-o",
        type=str,
        help="Output file",
        default=None,
        dest="output",
    )
    return parser


# --- Dataclasses
@dataclass
class Field:
    type: str
    name: str
    default: Optional[Union[str, int, float, bool]] = None


@dataclass
class EnumElement:
    name: str
    value: int
    comment: Optional[str] = ""


@dataclass
class Enum:
    name: str
    elems: List[EnumElement]


@dataclass
class Type:
    name: str
    fields: List[Field]
    comment: Optional[str] = ""


@dataclass
class Source:
    name: str
    includes: List[str]
    namespace: Optional[str] = None
    types: Optional[List[Type]] = None
    enums: Optional[List[Enum]] = None


ConfigSchema = class_schema(Source)


@dataclass
class Context:
    timestamp: str
    input_files: list[str]
    output_file: str
    template_file: str
    name: str
    includes: list[str]
    sources: list[Source]


def load_and_validate_yaml(yaml_file: str):
    try:
        with open(yaml_file, "r") as file:
            data = yaml.safe_load(file)
        schema = ConfigSchema()
        config = schema.load(data)
        return config

    except ValidationError as err:
        raise ValidationError(err.messages)

    except Exception as e:
        raise e


def prepare_context(input_files: list[str], template_file: str, output_file: str, extra_includes: list[str] = []):

    sources = [load_and_validate_yaml(input_file) for input_file in input_files]
    return Context(
        timestamp=datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        input_files=[os.path.basename(input_file) for input_file in input_files],
        output_file=os.path.basename(output_file),
        template_file=os.path.basename(template_file),
        name=utils.sanitize_string_for_cpp(os.path.basename(output_file).split(".")[0]),
        includes=list(
            set([include for source in sources for source in sources for include in source.includes] + extra_includes)
        ),
        sources=sources,
    )


def main():
    logger.setLevel(logging.DEBUG)
    stderr_handler = logging.StreamHandler(sys.stderr)
    stderr_handler.setLevel(logging.DEBUG)
    formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
    stderr_handler.setFormatter(formatter)
    logger.addHandler(stderr_handler)

    parser = build_parser()
    args = parser.parse_args()

    env = jinja2.Environment()
    utils.setup_env(env)

    try:
        if len(args.input_files) == 0:
            raise ValueError("No input files provided")

        context = prepare_context(args.input_files, args.template_file, args.output)

        with open(args.template_file, "r") as file:
            template = file.read()

        content = env.from_string(template).render(asdict(context))

        if args.output is not None:
            with open(args.output, "w") as file:
                file.write(content)
        else:
            print(content)

    except Exception as e:
        logger.error(e, exc_info=True)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
