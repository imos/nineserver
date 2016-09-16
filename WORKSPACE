http_archive(
    name = "gbase",
    url = "https://github.com/imos/gbase/releases/download/v0.8.4/gbase.zip",
    sha256 = "a4020f78f43962cf13ec85753809ee28749c7c6243c46337f5c441b364e98090",
)

bind(
    name = "base",
    actual = "@gbase//base"
)

bind(
    name = "testing",
    actual = "@gbase//base:testing"
)

bind(
    name = "testing_main",
    actual = "@gbase//base:testing_main"
)

git_repository(
    name = "cctz",
    remote = "https://github.com/google/cctz",
    tag = "v2.0",
)
