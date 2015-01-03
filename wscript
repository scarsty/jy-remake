#!/usr/bin/env python
# encoding: utf-8
# Author: Zgames <zgames@yeah.net>
# Waf build script for Heros of Jinyong.
# usage:
#   python waf configure build install


top = "."
out = "waf-build"

def options(ctx):
    ctx.load("gcc gxx winres")


def configure(ctx):
    ctx.load("gcc gxx winres")


def build(ctx):
    ctx.recurse(["source"])

