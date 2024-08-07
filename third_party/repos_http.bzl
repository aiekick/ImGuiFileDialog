"""Fetch external http repositories."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def fetch_http_repositories(name):
    """Fetch external http repositories.

    Args:
        name: unused.
    """

    # Make sure we follow the `docking` branch for multi-port and docking support!
    http_archive(
        name = "imgui",
        build_file = "//third_party/imgui:imgui.BUILD",
        sha256 = "d4b7fd185443111a3a892d4625c85ab9666c6c9cb5484e3a447de6af419f8d2f",
        strip_prefix = "imgui-1.90-docking",
        url = "https://github.com/ocornut/imgui/archive/refs/tags/v1.90-docking.tar.gz",
    )

