Import("env")

env.Append(LINKFLAGS=["--code-loc", "0x6C00"])
