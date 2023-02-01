/* ------------------------------------------------------------------   */
/*      item            : VSGXMLReader.hxx
        made by         : Rene van Paassen
        date            : 230201
        category        : header file
        description     :
        changes         : 230201 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#pragma once

#include <string>
#include "VSGViewer.hxx"

/** Read an XML file with type definitions and optionally static
    object instantiations. */
void VSGXMLRead(const std::string& file, VSGViewer &viewer);


