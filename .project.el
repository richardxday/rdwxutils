(c++-mode . ((filetypes . (cpp-project))
             (includes . ("redirector"))
             (flags . ("-xc++"
                       "-std=c++11"))
             (warnings . ("all"
                          "extra"
                          "everything"))
             (packages . ("rdlib-0.1"))
             (sources . ("src"
                         "../rdlib/include"
                         "../rdlib/src"))
             (buildcmd . "make -f makefile")))
