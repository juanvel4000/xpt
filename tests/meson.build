if get_option('ebt_only').disabled()
    test_manifests = executable(
        'test_manifests',
        'test_manifests.c',
        dependencies: [libxpt_dep],
        include_directories: include_directories('../libxpt'),
    )
    test_resolver = executable(
        'test_resolver',
        'test_resolver.c',
        dependencies: [libxpt_dep],
        include_directories: include_directories('../libxpt'),
    )

    test(
        'dependency resolution',
        test_resolver,
        workdir: meson.current_source_dir(),
    )
    test('manifest parsing', test_manifests, workdir: meson.current_source_dir())
endif
if get_option('ebt').enabled()
    test(
        'ebs checksum mismatch',
        find_program('test_ebs_checksum_mismatch.sh'),
        args: [
            meson.current_source_dir() / '../ebt/ebs.sh',
            meson.current_source_dir() / 'fixtures/corrupted_checksum.ebs.formula'
        ],
        timeout: 120,
    )
    test(
        'ebs formula build',
        find_program('test_ebs.sh'),
        args: [
            meson.current_source_dir() / '../ebt/ebs.sh',
            meson.current_source_dir() / '../docs/examples/ebs.formula',
        ],
        timeout: 120,
    )
endif
