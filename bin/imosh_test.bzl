def imosh_test(
    name,
    srcs=[],
    data=[],
    **kargs):
  if len(srcs) != 1:
    fail("Exactly one source file must be given.")
  native.genrule(
      name = name + "_genrule_sh",
      srcs = ["//bin:imosh_test_generate"],
      outs = [name + "_genrule.sh"],
      cmd = "$(BINDIR)/bin/imosh_test_generate " +
            PACKAGE_NAME + "/" + srcs[0] + " >$@",
  )
  native.sh_test(
      name = name,
      srcs = [name + "_genrule.sh"],
      data = [":" + srcs[0]] + data + ["//bin:imosh"],
      **kargs)
