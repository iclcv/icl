# Packaging

## Create packages/distributions

### Ubuntu

Currently, Ubuntu source and binary packages are supported. The first are built with `dh` while the later can also be generated with `CPack`.

```bash
packaging/scripts/ubuntu-build.sh  # configure package files
```

`ubuntu-build.sh` has to be called to create the `debian` folder in the source root which is required for the next steps. If you plan a custom build make sure to pass `-DBUILD_REDIST=DEB` to the `cmake` call.

#### Ubuntu source packages

Build source packages with:

```bash
debuild -S -i -I -sa
```

The resulting files will be places next to the icl source folder. `debuild` will ask for a PGP key to sign the package. See the *Preparing Upload* section for more details.

#### Working with launchpad

To upload the files to launchpad use `dput`:

```bash
dput ppa:iclcv/icl icl_10.0.0+$RELEASE$BUILDNR_source.changes
# dput ppa:iclcv/icl icl_10.0.0+trusty0_source.changes
```

#### Preparing Upload

Launchpad identifies packages by the used key to certify the package. Before you upload the changes, make sure your PGP key has been uploaded to the Ubuntu key server and assigned to your Launchpad profile.

```bash
gpg --gen-key  # create key if necessary
gpg --fingerprint  # show fingerprints
gpg --send-keys --keyserver keyserver.ubuntu.com $KEY  # send key to ubuntu server
gpg --output decrypted.txt --decrypt encrypted.gpg  # decrypt launchpad message
gpg --export-secret-key -a "User Name" > private.key  # for reuse
```

Exporting the key is only required when you used a docker image to build the package. If you do not export the key (into a shared host volume) it will be lost the moment you leave the image.

#### Build binaries from source (DH)

To build binary packages directly, use:

```bash
debuild -b -uc -us
```

Again, the DEB files will be placed right next to your ICL source folder.

#### Build binaries from source (CPACK)

Alternatively, you can use `CPack` to build binary Debian packages. These packages will be created in your build folder.

```bash
make doc package
```

Currently, `CPack` builds have no advantages compared to `dh` builds. However,
`CPack` also features binary installer builds for Linux, Windows and OSX.

#### Docker test build environment

```bash
cd packaging/docker/trusty
docker build -t icl .
docker run -ti --mount type=bind,source=${LOCAL_WORKSPACE},target=/home/user/workspace icl:latest
```

### Windows

To build the Windows 'WIX' installer, [pthreads-win32](https://www.sourceware.org/pthreads-win32/) needs to be available.
It is recommended to build the library from source with the ICL's build environment.
Using the prebuilt libraries makes the ICL dependent on the Visual Studio 10 runtime environment (in addition to the targeted environment) which can be prevented by linking the
pthreadVC2.dll against the current runtime.

The following batch script builds the msi installer for Visual Studio 2017 Win64:

```bash
packaging/scripts/windows-build.bat <pthreads_root>
```
