/*
 * Go's prebuilt race object declares a macOS 10.7 minimum even though the
 * interfaces it needs are supplied by this port's Snow Leopard shim. Rewrite
 * only that declaration so Xcode 4.2 accepts otherwise compatible programs.
 */

#include <mach-o/loader.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MACOS_10_6_VERSION 0x000a0600U
#define MACOS_10_7_VERSION 0x000a0700U
#define SNOW_LC_VERSION_MIN_MACOSX 0x24U

/* Snow Leopard's loader.h predates this command, although its linker emits it. */
struct snow_version_min_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t version;
    uint32_t sdk;
};

static void
fail(const char *message, const char *path)
{
    fprintf(stderr, "patch-macho-min-version: %s: %s\n", path, message);
    exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
    struct mach_header_64 header;
    struct load_command command;
    struct snow_version_min_command minimum;
    FILE *file;
    uint32_t index;

    if (argc != 2)
        fail("expected one Mach-O object path", argc > 1 ? argv[1] : "(missing)");

    file = fopen(argv[1], "r+b");
    if (file == NULL)
        fail("cannot open object", argv[1]);
    if (fread(&header, sizeof(header), 1, file) != 1 || header.magic != MH_MAGIC_64)
        fail("not a native 64-bit Mach-O object", argv[1]);

    for (index = 0; index < header.ncmds; index++) {
        long offset = ftell(file);

        if (offset < 0 || fread(&command, sizeof(command), 1, file) != 1)
            fail("truncated load-command table", argv[1]);
        if (command.cmdsize < sizeof(command))
            fail("invalid load-command size", argv[1]);
        if (command.cmd == SNOW_LC_VERSION_MIN_MACOSX) {
            if (command.cmdsize != sizeof(minimum) ||
                    fseek(file, offset, SEEK_SET) != 0 ||
                    fread(&minimum, sizeof(minimum), 1, file) != 1)
                fail("invalid minimum-version command", argv[1]);
            if (minimum.version != MACOS_10_7_VERSION)
                fail("unexpected original minimum version", argv[1]);

            minimum.version = MACOS_10_6_VERSION;
            if (fseek(file, offset, SEEK_SET) != 0 ||
                    fwrite(&minimum, sizeof(minimum), 1, file) != 1 ||
                    fclose(file) != 0)
                fail("cannot update minimum version", argv[1]);
            return EXIT_SUCCESS;
        }
        if (fseek(file, offset + command.cmdsize, SEEK_SET) != 0)
            fail("invalid load-command offset", argv[1]);
    }

    fail("minimum-version command not found", argv[1]);
    return EXIT_FAILURE;
}
