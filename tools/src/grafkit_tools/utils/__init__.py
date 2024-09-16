import re


def hex_byte(value):
    return f"0x{value:02x}"


def generate_chunks(data, chunk_size=16):
    for i in range(0, len(data), chunk_size):
        chunk = data[i : i + chunk_size]
        hex_bytes = [f"0x{byte:02x}" for byte in chunk]
        yield {"offset": i, "bytes": hex_bytes}


def sanitize_string_for_cpp(name):
    sanitized_name = re.sub(r"[^a-zA-Z0-9_]", "_", name)
    if re.match(r"^\d", sanitized_name):
        sanitized_name = "_" + sanitized_name
    return sanitized_name


def snake_to_camel_case(name):
    return "".join(word.title() for word in name.split("_"))


def setup_env(env):
    env.filters["hex_byte"] = hex_byte
    env.filters["generate_chunks"] = generate_chunks
    env.filters["sanitize_for_cpp"] = sanitize_string_for_cpp
    env.filters["snake_to_camel_case"] = snake_to_camel_case
    return env
