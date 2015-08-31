# Documentation: https://github.com/Homebrew/homebrew/blob/master/share/doc/homebrew/Formula-Cookbook.md
#                http://www.rubydoc.info/github/Homebrew/homebrew/master/frames
# PLEASE REMOVE ALL GENERATED COMMENTS BEFORE SUBMITTING YOUR PULL REQUEST!

class Icl < Formula
  desc "Image Component Library"
  homepage "http://www.iclcv.org"
  version "osx"
  url "https://opensource.cit-ec.de/svn/icl/branches/osx", :using => :svn
  head "https://opensource.cit-ec.de/svn/icl/branches/osx", :using => :svn
  sha256 ""

  depends_on "cmake" => :build
  depends_on "glew"
  depends_on "jpeg"
  depends_on "libpng"
  depends_on "qt5"
  depends_on "protobuf"
  depends_on "rsb"
  depends_on "opencv"
  depends_on "libdc1394"

  #depends_on :x11 # if your formula requires any X11/XQuartz components

  def install
    args = std_cmake_args
    # we have to work around the cellar structure and the plugin load path here
    args.reject{ |item| item =~ /CMAKE_INSTALL_PREFIX/i}
    args = args + %W[
      -DBUILD_APPS=On
      -DBUILD_DEMOS=On
      -DBUILD_EXAMPLES=On
      -DBUILD_WITH_OPENGL=On
      -DBUILD_WITH_QT=ON
      -DBUILD_WITH_PROTOBUF=On
      -DBUILD_WITH_RSB=On 
      -DBUILD_WITH_OPENCV=On 
      -DBUILD_WITH_IMAGEMAGICK=On
      -DBoost_USE_STATIC_LIBS=On
      -DCMAKE_PREFIX_PATH='#{HOMEBREW_PREFIX}/Cellar/qt5/5.4.1'
      -DBOOST_ROOT='#{HOMEBREW_PREFIX}'
      -DCMAKE_INSTALL_PREFIX='#{HOMEBREW_PREFIX}'
    ]
    system "cmake", ".", *args
    system "make DESTDIR=#{prefix} install"
    mv Dir["#{prefix}/usr/local/*"], "#{prefix}"
  end

  test do
    # `test do` will create, run in and delete a temporary directory.
    #
    # This test will fail and we won't accept that! It's enough to just replace
    # "false" with the main program this formula installs, but it'd be nice if you
    # were more thorough. Run the test with `brew test icl`. Options passed
    # to `brew install` such as `--HEAD` also need to be provided to `brew test`.
    #
    # The installed folder is not in the path, so use the entire path to any
    # executables being tested: `system "#{bin}/program", "do", "something"`.
    system "false"
  end
end
