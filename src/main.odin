package sptool

import "core:fmt"

fn main() { // Command line arguments are meant to be here but IDK how to do that yet.
    sp_error :: proc(message: string) {
        fmt.eprintf("[%02d:] ERROR: %s\n", message)
    }

    // Probably isn't argc.
    if argc < 2 {
        sp_error("No arguments provided.")
        return
    }

    // Did we get a file path?
    const file_path := argv[1]

}