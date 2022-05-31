#include <cstdio>
#include <iostream>
#include <string>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>

#include "code_nodes.hpp"


namespace smart {

    static const char *self_text(SimpleTextNodeStruct *self) {
        return self->text;
    }

    static st_textlen selfTextLength(SimpleTextNodeStruct *self) {
        return self->textLength;
    }

    static CodeLine *appendToLine(SimpleTextNodeStruct *self, CodeLine *currentCodeLine) {
        return currentCodeLine->addPrevLineBreakNode(self)->appendNode(self);
    }

    static constexpr const char simpleTextTypeText[] = "<SimpleText>";

    static struct node_vtable simpleTextVTABLE = CREATE_VTABLE2(SimpleTextNodeStruct,
                                                                selfTextLength,
                                                                self_text,
                                                                appendToLine, simpleTextTypeText
                                                                  , NodeTypeId::SimpleText, 0);
    const struct node_vtable *const VTables::SimpleTextVTable = &simpleTextVTABLE;





    static constexpr const char spaceTextTypeText[] = "<SpaceText>";

    static struct node_vtable _spaceVTable = CREATE_VTABLE2(SpaceNodeStruct,
                                                            selfTextLength,
                                                            self_text,
                                                            appendToLine, spaceTextTypeText, NodeTypeId::Space, 1
    );
    const struct node_vtable *const VTables::SpaceVTable = &_spaceVTable;


    static struct node_vtable _nullVTable = CREATE_VTABLE2(NullNodeStruct,
                                                           selfTextLength,
                                                           self_text,
                                                           appendToLine, "<NULL>", NodeTypeId::NULLId, 2
    );
    const struct node_vtable *const VTables::NullVTable = &_nullVTable;


    SimpleTextNodeStruct *
    Alloc::newSimpleTextNode(ParseContext *context, NodeBase *parentNode) {
        auto *spaceNode = context->newMemForNode<SimpleTextNodeStruct>();
        auto *node = Cast::upcast(spaceNode);

        INIT_NODE(node, context, parentNode, VTables::SimpleTextVTable);
        return spaceNode;
    }


    SpaceNodeStruct *Alloc::newSpaceNode(ParseContext *context, NodeBase *parentNode) {
        auto *spaceNode = context->newMemForNode<SpaceNodeStruct>();
        auto *node = Cast::upcast(spaceNode);

        INIT_NODE(node, context, parentNode, VTables::SpaceVTable);
        return spaceNode;
    }

    NullNodeStruct *Alloc::newNullNode(ParseContext *context, NodeBase *parentNode) {
        auto *nullNode = context->newMemForNode<NullNodeStruct>();
        auto *node = Cast::upcast(nullNode);

        INIT_NODE(node, context, parentNode, VTables::NullVTable);
        return nullNode;
    }
}