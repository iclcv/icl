####################################
Image Component Library (ICL) Manual
####################################

.. |A| image:: icons/92px/utils.png
       :target: modules/utils.html
      
.. |B| image:: icons/92px/math.png
       :target: modules/math.html

.. |C| image:: icons/92px/core.png
       :target: modules/core.html

.. |D| image:: icons/92px/filter.png
       :target: modules/filter.html

.. |E| image:: icons/92px/io.png
       :target: modules/io.html

.. |F| image:: icons/92px/qt.png
       :target: modules/qt.html

.. |G| image:: icons/92px/cv.png
       :target: modules/cv.html

.. |H| image:: icons/92px/geom.png
       :target: modules/geom.html

.. |I| image:: icons/92px/markers.png
       :target: modules/markers.html

.. |TT| image:: icons/tooltip.png
.. |ITT| image:: icons/index-tip.png

.. |ABOUT| image:: icons/about.png
             :target: extras/about.html

.. |DOWNLOAD| image:: icons/download.png
                :target: extras/download.html

.. |HOWTOS| image:: icons/howtos.png
              :target: extras/howtos.html

.. |INSTALL| image:: icons/install.png
               :target: extras/install.html

.. |TUTORIAL| image:: icons/tutorial.png
                :target: extras/tutorial.html

.. |H1| image:: icons/h1.png

.. |H2| image:: icons/h2.png

.. |H3| image:: icons/h3.png

.. |H4| image:: icons/h4.png


.. we force the 3 column layout here!

**Modules**

  | |A| |B| |C|
  | |D| |E| |F|
  | |G| |H| |I|

**Getting Started**

  | |ABOUT|   |DOWNLOAD|   |INSTALL|   |TUTORIAL|   |HOWTOS|


Contents
********

.. toctree::
   :maxdepth: 1

   extras/about
   extras/tutorial
   extras/howtos
   extras/download
   extras/install


Internal
********

.. toctree::
   :hidden:
      
   js.rst
   modules/utils.rst
   modules/math.rst
   modules/core.rst
   modules/filter.rst
   modules/io.rst
   modules/qt.rst
   modules/cv.rst
   modules/geom.rst
   modules/markers.rst

.. toctree::
   :maxdepth: 1

   extras/todolist

.. Indices and tables
   ==================
     * :ref:`genindex`
     * :ref:`modindex`
     * :ref:`search`



.. raw:: html
  
  <script src="js/jquery.min.js"></script>
  <script src="js/jquery.tools.min.js"></script>
  <!--script src="../_static/underscore.js"></script-->
         <!--script src="../_static/doctools.js"></script-->


  <script>

  $(document).ready(function() {

    var f = function(){

       var href = this.href;
       var id = "";
       if(href.match('\.*about.html$')){
          id='extras-about';
       }else if(href.match('.*download\.html$')){
          id='extras-download';
       }else if(href.match('.*install\.html$')){
          id='extras-install';
       }else if(href.match('.*tutorial\.html$')){
          id='extras-tutorial';
       }else if(href.match('.*howtos\.html$')){
          id='extras-howtos';
       }
       return '<div id="'+id+'" class="tooltip extras"></div>'
    }
  
    var extras = [ 'about', 'download', 'install', 'tutorial', 'howtos' ];
    for(var i=0;i<5;++i){
      var e = $('.reference.external[href="extras/'+extras[i]+'.html"]');
      e.after(f);
      e.tooltip({
        position: "top center",
        opacity: 0.95,
        effect: 'fade',
        predelay: 1,
        delay: 10,
        fadeInSpeed: 300,
        offset: [5,-5]
      });
    }

    $('.reference.external[href^="modules"]').after(function() {
       var href = this.href;
       var id = "";
       var text = "not defined yet";
       if(href.match('.*utils.*')){
         id = "module-utils";
         text = "the <b>utils</b> module provides basic data type definitions as well as general support types and functions";
       }else if(href.match('.*math.*')){
         id = "module-math";
         text = "the <b>math</b> module provides fixed- and dynamic sized matrix classes and several machine learning tools";
       }else if(href.match('.*core.*')){
         id = "module-core";
         text = "the <b>core</b> module provides the basic image classes <b>ImgBase</b> and <b>Img</b>, basic types and functions";
       }else if(href.match('.*filter.*')){
         id = "module-filter";
         text = "the <b>filter</b> module provides a huge set of unary- and binary image filters for low level image processing";
       }else if(href.match('.*qt.*')){
         id = "module-qt";
         text = "the <b>qt</b> module provides a powerfull GUI creation framework including high-speed visualization tools";
       }else if(href.match('.*cv.*')){
         id = "module-cv";
         text = "the <b>cv</b> module provides a huge set of medium level classes and functions for computer vision";
       }else if(href.match('.*geom.*')){
         id = "module-geom";
         text = "the <b>geom</b> module provides 3D-vision and point-cloud processig tools including 3D-visualization";
       }else if(href.match('.*markers.*')){
         id = "module-markers";
         text = "the <b>markers</b> module provides a generic fiducial marker detection frameworks for common marker types";
       }else if(href.match('.*io.*')){
         id = "module-io";
         text = "the <b>io</b> module provides an image acquisition  and output framework for most differnt in- and output types";
       }
       return '<div id="'+id+'"class="tooltip modules">'+text+'</div>';
    });

    
    $('.reference.external[href^="modules"]').tooltip({
      position: "top center", 
      opacity: 1,
      effect: 'fade',
      predelay: 300,
      delay: 10,
      fadeInSpeed: 1000,
      offset: [36,10]
    });

    $('#module-utils').bind('click',function(e){ location = 'modules/utils.html';  });
    $('#module-math').bind('click',function(e){ location = 'modules/math.html';  });
    $('#module-core').bind('click',function(e){ location = 'modules/core.html';  });
    $('#module-filter').bind('click',function(e){ location = 'modules/filter.html';  });
    $('#module-io').bind('click',function(e){ location = 'modules/io.html';  });
    $('#module-qt').bind('click',function(e){ location = 'modules/qt.html';  });
    $('#module-cv').bind('click',function(e){ location = 'modules/cv.html';  });
    $('#module-geom').bind('click',function(e){ location = 'modules/geom.html';  });
    $('#module-markers').bind('click',function(e){ location = 'modules/markers.html';  });

    $('#extras-about').bind('click',function(e){ location = 'extras/about.html';  });
    $('#extras-download').bind('click',function(e){ location = 'extras/download.html';  });
    $('#extras-install').bind('click',function(e){ location = 'extras/install.html';  });
    $('#extras-tutorial').bind('click',function(e){ location = 'extras/tutorial.html';  });
    $('#extras-howtos').bind('click',function(e){ location = 'extras/howtos.html';  });

  });
  </script>
  <style type="text/css"> 
    .tooltip.modules {
      display:none;
      background: transparent url(_images/index-tip.png);
      font-size:12px;
      height:100px;
      width:160px;
      padding: 10px;
      padding-right:30px;
      padding-left:100px;
      line-height: 20px;
      color: rgb(70,70,70);
    }
    
    .tooltip.extras {
      display:none;
      background: rgba(0,100,255,0.2);
      font-size:13px;
      height:88px;
      width:99px;
      padding:1px;
      color: #555;
      line-height: 20px;
      border-radius: 17px;
    }

    div.body{
      border-top-left-radius: 15px;
      border-bottom-left-radius: 15px;
      border: 1px solid rgb(110,110,110);
      box-shadow: 0px 0px 50px rgba(0,0,0,0.7);
    }

    div.body h1{
      border-top-left-radius: 15px;
      box-shadow: 0px 2px 0px rgba(0,0,0,0.4);
    }

    div.body h2{
      box-shadow: 0px 2px 0px rgba(0,0,0,0.4);
    }

    div.body h3{
      box-shadow: 0px 2px 0px rgba(0,0,0,0.4);
    }

    div.sphinxsidebar{
      font-size: 80%;
    }
    div.body h1, div.body h2, div.body h3, div.body h4{
      margin-bottom: 0px;
      background-repeat: no-repeat;
      margin-left: -41px;
      box-shadow: none;
      border: none;
      opacity: 1;
      color: rgb(230,230,230);
      height: 44px;
      padding-top: 4px;
    }

    div.body h1{
      padding-top: 7px;
      background: transparent url(_images/h1.png);
      margin-top: 8px;
      border-radius: 0px;
      height: 57px;
      padding-top: 3px;
    }

    div.body h2{
      padding-top: 7px;
      background: transparent url(_images/h2.png);
      height: 52px;
    }

    div.body h3{
      background: transparent url(_images/h3.png);
    }

    div.body h4{
      padding-top: 5px;
      height: 42px;
      background: transparent url(_images/h4.png);
      color: rgb(60,60,60);
    }

    div.sphinxsidebar{
      font-size: 80%;
    }

    table.docutils td, table.docutils th{
      border: 0px;
    }
    th {
       background-color: #0F67A1;
       color: rgb(220,220,220);
    }

    img[alt="shadow"]{
       box-shadow: 5px 5px 12px rgba(0,0,0,0.3);
    }

    a.headerlink {
       color: rgb(230,230,230);
    }
    a.headerlink:hover{
       color: white;
       background: transparent;
    }

    h4:hover > a.headerlink {
       color: rgb(60,60,60);
    }
    h4 > a.headerlink {
       color: rgb(90,90,90);
    }
  
    </style>
