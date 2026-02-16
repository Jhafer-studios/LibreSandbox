const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "LibreSandbox",
        .root_source_file = .{ .path = "engine_core.c" },
        .target = target,
        .optimize = optimize,
    });

    exe.linkLibC();
    exe.addIncludePath(.{ .path = "lib" });
    exe.addIncludePath(.{ .path = "include" });

    exe.linkSystemLibrary("glfw3");
    exe.linkSystemLibrary("opengl32");
    exe.linkSystemLibrary("gdi32");
    exe.linkSystemLibrary("user32");
    exe.linkSystemLibrary("shell32");

    var src_dir = std.fs.cwd().openDir("src", .{ .iterate = true }) catch unreachable;
    var iter = src_dir.iterate();
    while (iter.next() catch unreachable) |file| {
        if (std.mem.endsWith(u8, file.name, ".c")) {
            const path = b.fmt("src/{s}", .{file.name});
            exe.addCSourceFile(.{ .file = .{ .path = path }, .flags = &[_][]const u8{"-O3"} });
        }
    }

    b.installArtifact(exe);
}