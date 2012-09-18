#########################
Welcome to the ICL Manual
#########################

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

.. we force the 3 column layout here!

| |A| |B| |C|
| |D| |E| |F|
| |G| |H| |I|


Contents
********
.. toctree::
   :maxdepth: 1

   about
   modules
   tutorial
   howtos


Internal
********

.. toctree::
   :hidden:
      
   js.rst

.. toctree::
   :maxdepth: 1

   todolist

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
    $('.reference.external').after(function() {
       var href = this.href;
       var text = "not defined yet";
       if(href.match('.*utils.*')){
         text = "the <b>utils</b> module provides basic data type definitions as well as general support types and functions";
       }else if(href.match('.*math.*')){
         text = "the <b>math</b> module provides fixed- and dynamic sized matrix classes and several machine learning tools";
       }else if(href.match('.*core.*')){
         text = "the <b>core</b> module provides the basic image classes <b>ImgBase</b> and <b>Img</b>, basic types and functions";
       }else if(href.match('.*filter.*')){
         text = "the <b>filter</b> module provides a huge set of unary- and binary image filters for low level image processing";
       }else if(href.match('.*qt.*')){
         text = "the <b>qt</b> module provides a powerfull GUI creation framework including high-speed visualization tools";
       }else if(href.match('.*cv.*')){
         text = "the <b>cv</b> module provides a huge set of medium level classes and functions for computer vision";
       }else if(href.match('.*geom.*')){
         text = "the <b>geom</b> module provides 3D-vision and point-cloud processig tools including 3D-visualization";
       }else if(href.match('.*markers.*')){
         text = "the <b>markers</b> module provides a generic fiducial marker detection frameworks for common marker types";
       }else if(href.match('.*io.*')){
         text = "the <b>io</b> module provides an image acquisition  and output framework for most differnt in- and output types";
       }
       return '<div class="tooltip">'+text+'</div>';
    });

    
    $('.reference.external ').tooltip({
      position: "top center", 
      opacity: 1,
      effect: 'fade',
      predelay: 300,
      delay: 10,
      fadeInSpeed: 1000,
      offset: [36,10]
    });

  });
  </script>
  <style type="text/css"> 
    .tooltip {
      display:none;
       background: transparent url(_images/index-tip.png);
      font-size:13px;
      height:100px;
      width:160px;
      padding: 10px;
      padding-right:30px;
      padding-left:100px;
      line-height: 20px;
      color: rgb(50,50,50);
    }
  </style>
