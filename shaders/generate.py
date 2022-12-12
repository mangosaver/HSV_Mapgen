def dump_as_c_str(varname, filename):
    out = "const char* " + varname + " = "

    with open(filename) as file:
        lines = file.read().split("\n")
        for i, line in enumerate(lines):
            if line == "" or line == "\n":
                continue
            if i == len(lines) - 1:
                out += ("\"" + line + "\";\n")
            else:
                out += ("\"" + line + "\\n\"\n")
    return out

with open("generated.h", "w+") as embeddedFile:
    embeddedFile.write(dump_as_c_str("vertSrc", "vert.glsl"))
    embeddedFile.write(dump_as_c_str("fragSrc", "frag.glsl"))
#
# with open("frag.glsl") as fragSrc:
#
#     src = fragSrc.read()
#     lines = src.split("\n")
#     test = "".join((line + "\n") for line in lines)
#
#     out = "const char* fragSrc = "
#     for i, line in enumerate(lines):
#         if line == "" or line == "\n":
#             continue
#         if i == len(lines) - 1:
#             out += ("\"" + line + "\";\n")
#         else:
#             out += ("\"" + line + "\\n\"\n")
#     print(out)
