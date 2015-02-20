#!/usr/bin/env python
# encoding: utf-8
# Author: Zgames <zgames@yeah.net>
# Waf build script for Heros of Jinyong.
# usage:
#   python waf configure build install


top = "."
out = "waf-build"

def options(ctx):
    ctx.add_option("--sdl-version", default="2.0", action="store", 
            help="can be 1.2 or 2.0")
    ctx.load("gcc gxx winres")


def configure(ctx):
    if ctx.options.sdl_version not in ("1.2","2.0"):
        ctx.fatal("invalid SDL version")
    cwd = ctx.path.abspath()
    env = ctx.env
    env.INSTALL_PATH = cwd + "/release"
    env.PROJECT_ROOT = cwd
    env.SDK_PATH = cwd + "/sdk"
    env.SDL_VERSION = ctx.options.sdl_version
    if env.SDL_VERSION == "1.2":
        env.SDL_INCLUDE_PATH = cwd + "/sdk/SDL/include"
        env.SDL_LIB_PATH = cwd + "/sdk/SDL/lib"
    elif env.SDL_VERSION == "2.0":
        env.SDL_INCLUDE_PATH = cwd + "/sdk/SDL2/include"
        env.SDL_LIB_PATH = cwd + "/sdk/SDL2/lib"
    ctx.load("gcc gxx winres")


def build(ctx):
    ctx.recurse(["source"])

