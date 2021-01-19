#ifndef SQUAREPINE_JAVASCRIPT_H
#define SQUAREPINE_JAVASCRIPT_H

/** BEGIN_JUCE_MODULE_DECLARATION

    ID:                 squarepine_javascript
    vendor:             SquarePine
    version:            0.1.0
    name:               SquarePine Javascript
    description:        A decent Javascript interpreter that is trying to be compliant.
    website:            https://www.squarepine.io
    license:            Proprietary
    minimumCppStandard: 11
    dependencies:       juce_data_structures

    END_JUCE_MODULE_DECLARATION
*/

//==============================================================================
#include <regex>
#include <random>
#include <sstream>
#include <locale>
#include <iomanip>

#include <juce_data_structures/juce_data_structures.h>

#if JUCE_MODULE_AVAILABLE_juce_gui_extra
    #include <juce_gui_extra/juce_gui_extra.h>
#endif

//==============================================================================
namespace sp
{
    using namespace juce;

    #include "core/squarepine_RFC2822Time.h"
    //#include "core/squarepine_AST.h"
    //#include "core/squarepine_Lexer.h"

    #include "core/squarepine_RootObject.h"
    #include "core/squarepine_JavascriptEngine.h"

   #if JUCE_MODULE_AVAILABLE_juce_gui_extra
    #include "graphics/squarepine_JavascriptCodeTokeniser.h"
   #endif
}

#endif //SQUAREPINE_JAVASCRIPT_H
