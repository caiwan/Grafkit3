from setuptools import setup, find_namespace_packages

with open("README.MD", "r") as fh:
    long_description = fh.read()

install_requires = [
    "Jinja2==3.1.4",
    "PyYAML==6.0.2",
    "dataclasses-json==0.5.7",
    "more-itertools==8.12.0",
]

test_requires = [
    "flake8==7.1.1",
    "pytest==8.3.2",
]

setup(
    name="grakfit_tools",
    version="0.0.1",
    description="Source utilities for Grafkit",
    long_description=long_description,
    long_description_content_type="text/markdown",
    license="MIT",
    license_files=("LICENSE",),
    classifiers=[
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
    ],
    packages=find_namespace_packages(where="src", exclude=["tests", "*.tests", "tests.*", "*.tests.*"]),
    install_requires=install_requires,
    include_package_data=True,
    package_dir={"": "src"},
    setup_requires=[
        # "setuptools>=74.1.2",
        # "wheel"
    ],
    zip_safe=False,
    entry_points={
        "console_scripts": [
            "hexdump=grafkit_tools.hexdump:main",
        ],
    },
    extras_require={"test": test_requires},
)
