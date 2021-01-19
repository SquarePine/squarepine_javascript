#include "squarepine_javascript.h"

namespace sp
{
    using namespace juce;

    #include "core/squarepine_RFC2822Time.cpp"
    #include "core/squarepine_Parsing.h"
    #include "core/squarepine_Classes.h"
    #include "core/squarepine_RootObject.cpp"
    #include "core/squarepine_JavascriptEngine.cpp"

   #if JUCE_MODULE_AVAILABLE_juce_gui_extra
    #include "graphics/squarepine_JavascriptCodeTokeniser.cpp"
   #endif
}
