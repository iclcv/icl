# Packaging

## Create packages/distributions

### Ubuntu

Currently, Ubuntu source and binary packages are supported. The first are built with `dh` while the later can also be generated with `CPack`.

#### Docker

Currently, docker images for `trusty`, `xenial` and `artful` are provided.
All of them use `build-ubuntu-packages.sh` which can be found in `script` and
automate the build for binary packages or packaging and deployment for source packages.
The steps required to do so will be describes in the following sections in case more
information is required.

*IMPORTANT* All of them expect the ICL source code in `${LOCAL_WORKSPACE}/icl`!
Additionally, compiled binary packages will be copied to `${LOCAL_WORKSPACE}`.
In case of source builds, there need to be two additional files in the workspace:

* `${LOCAL_WORKSPACE}/packaging.key` -- the **private** keychain to sign the packages
* `${LOCAL_WORKSPACE}/packaging_passphrase.txt` -- the passphrase of this keychain

It is advices to not use a master keychain here.
After the source package has been built it will automatically be uploaded to launchpad.

Example:

```bash
cd ${LOCAL_WORKSPACE}/icl/packaging/docker/trusty
# build the docker image first
docker build -t icl-trusty .
# build binary packages [default]
docker run -ti --mount type=bind,source=${LOCAL_WORKSPACE},target=/home/user/workspace icl-trusty
# build source packages
docker run -ti --mount type=bind,source=${LOCAL_WORKSPACE},target=/home/user/workspace -e variant=source icl-trusty
```

The version of the source package depends on the topmost entry in `scripts/packaging/debian/changelog.in`.
During `cmake`, required packages will be downloaded. To speed up this process in the
next run, the docker image can be extended with the newly created software layer.
The required container ID has been stored into `${LOCAL_WORKSPACE}/docker_container_id.log`

```bash
# add new layer
docker commit $(cat ${LOCAL_WORKSPACE}/docker_container_id.log) icl-trusty
# run docker interactively by passing '/bin/bash' AFTER the image name
docker run -ti --mount type=bind,source=${LOCAL_WORKSPACE},target=/home/user/workspace icl-trusty /bin/bash
```

#### Manual build

In case more control is required, all steps can be done interactively.
First, `ubuntu-build.sh` has to be called to create the `debian` folder in the source root which is required for the next steps. This will also attempt to install all need packages for
the following build process.

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

### Windows

To build the Windows 'WIX' installer, [pthreads-win32](https://www.sourceware.org/pthreads-win32/) needs to be available.
It is recommended to build the library from source with the ICL's build environment.
Using the prebuilt libraries makes the ICL dependent on the Visual Studio 10 runtime environment (in addition to the targeted environment) which can be prevented by linking the
pthreadVC2.dll against the current runtime.

The following batch script builds the msi installer for Visual Studio 2017 Win64:

```bash
packaging/scripts/windows-build.bat <pthreads_root>
```

### OSX

From [here](https://github.com/Homebrew/brew/blob/master/docs/Bottles.md):

Bottles are produced by installing a formula with `brew install --build-bottle <formula>` and then bottling it with `brew bottle <formula>`. This outputs the bottle DSL which should be inserted into the formula file.

Bottles have a DSL to be used in formulae which is contained in the `bottle do ... end` block.
A simple (and typical) example:

```ruby
bottle do
  sha256 "..." => :sierra
  sha256 "..." => :el_capitan
  sha256 "..." => :yosemite
end
```

An ICL bottle can be created with `brew install --build-bottle icl`, followed by `brew bottle icl`. The output has to be added to https://github.com/iclcv/homebrew-formulas while the bottle itself is added to the related tag/release.
The path to the bottle needs to be added to the icl formula as well as the new `root_url` ([ref](https://docs.brew.sh/Bottles.html)). This should be along the line of `https://github.com/iclcv/icl/releases/download/<version>`.
