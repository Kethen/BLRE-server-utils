set(PROXY_VERSION "v1.0.0-beta.2" CACHE STRING "version of the proxy library")

CPMAddPackage(
    NAME proxy
    VERSION ${PROXY_VERSION}
    GIT_REPOSITORY https://gitlab.com/blrevive/tools/proxy
    GIT_TAG v1.0.0-beta.2
)
