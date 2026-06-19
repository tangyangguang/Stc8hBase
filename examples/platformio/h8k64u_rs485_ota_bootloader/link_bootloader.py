Import("env")

env.Append(LINKFLAGS=["--code-loc", "0xB800"])
