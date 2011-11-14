#include "semanticvalidation.h"
#include <assert.h>


namespace nclValidator {

/*
 * Test reference of any scope */
void anyScopeReferenceValidation (const ModelElement &el, const ModelElement &pointed,
                                  const Attribute &att, vector<pair<void *, string> > &msg) {

        // Test 'refer' special case
        if (att.name() == "refer") {

                // 'refer' can't point to an element that defined a 'refer' att too
                if (pointed.attribute("refer").name() != ""){
                        fprintf (stderr, "'refer' attribute of '%s' element can't point to an element that have a 'refer' defined too\n",
                                        el.elementName().c_str());

                }

                // 'refer' can't point to an ancestor.
                if (pointed.id() == el.scope()){
                        fprintf (stderr, "'refer' attribute of '%s' element can't point to an ancestor\n",
                                        el.elementName().c_str());
                }
        }
}


/*
 *
 */
void specificScopeReferenceValidation (const ModelElement &el, const ModelElement &pointed, const Attribute &att,
                const ReferenceStructure &ref, Model &model, vector<pair<void *, string> > &msgs)
{
        const ModelElement *scopeElement = NULL;

        //////////////////////////////////////////////////////////////////////////////////////////
        //						RETRIEVE THE ELEMENT THAT DEFINE THE SCOPE 						//
        //////////////////////////////////////////////////////////////////////////////////////////

        // Scope is defined by an attribute of 'el'
        if (ref.getPerspective() == "$THIS")
                scopeElement = &el;

        // Scope is defined by an attribute of parent
        else if (ref.getPerspective() == "$PARENT")
                scopeElement = model.element(el.parent());

        // Scope is defined by an attribute of grandparent
        else if (ref.getPerspective() == "$GRANDPARENT") {
                scopeElement = model.element(el.parent());

                // scopeElement can be NULL here. This is why have this condition
                scopeElement = scopeElement ? model.element (scopeElement -> parent()) : scopeElement;
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        //						TEST THE SCOPE ELEMENT											//
        //////////////////////////////////////////////////////////////////////////////////////////

        // Test if scope was defined
        if (!scopeElement) {
                fprintf (stderr, "Unknown scope of '%s' attribute of '%s' element\n",
                                att.name().c_str(), el.elementName().c_str());
                return;
        }

        // Test scope att
        string scopeAtt = scopeElement -> attribute (ref.getPerspectiveAtt()).name();
        if (scopeAtt.empty()) {
                fprintf (stderr, "Unknown scope of '%s' attribute of '%s' element\n",
                                att.name().c_str(), el.elementName().c_str());
                return;
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        //						VALIDATE THE REFERENCE WITH THE SCOPE							//
        //////////////////////////////////////////////////////////////////////////////////////////

        // Retrieve the element pointed by $ELEMENT.att
        vector<ModelElement *> aux = model.elementByIdentifier (scopeAtt);
        if (aux.empty ()) {
                fprintf (stderr, "Unknown scope of '%s' attribute of '%s' element\n",
                                att.name().c_str(), el.elementName().c_str());
                return;
        }

        // Test if 'pointed.scope' is 'scopeElement.id'
        scopeElement = aux.front();
        if (pointed.scope() != scopeElement -> id()) {
                fprintf (stderr, "'%s' attribute of '%s' element should point to '%s' '%s'\n",
                        att.name().c_str(), el.elementName().c_str(),
                        scopeAtt.c_str(), scopeElement -> elementName().c_str());
                return;
        }
}


/*
 *
 */
void referenceValidation (const ModelElement &el, const Attribute &att, Model &model, vector<pair<void *, string> > &msgs,
                          Message& messageFactory)
{
        vector<ModelElement *> pointedElements = model.elementByIdentifier(att.value());

        if (att.name() == "interface" && pointedElements.size() == 0){
            pointedElements = model.elementByPropertyName(el.attribute("component").value(), att.value());
        }

        ///////////////////////////////////////////////////////////////////////////
        //  Um elemento que faz referncia, sempre ter que fazer refer?ncia,
        //  ou ele pode assumir um outro valor que no a referncia?
        ///////////////////////////////////////////////////////////////////////////

        // Elements points to nobody
        if (pointedElements.empty()) {
                fprintf (stderr, "'%s' attribute of '%s' element point to anybody\n",
                                att.name().c_str(), el.elementName().c_str());
                msgs.push_back(pair<void *, string> (el.data(),
                                                    messageFactory.createMessage(3003, 2, att.name().c_str(),
                                                                                 el.elementName().c_str())));
                model.addElementWithErrorInLastPass(el.id());
                return;
        }

        ModelElement *pointed = pointedElements.front();
//        assert (pointed);
        if (!pointed) return;

        const ReferenceStructure *ref = Langstruct::getReference(el.elementName(), att.name(), pointed -> elementName());

        /* Test if 'pointed' element can be pointed by 'el'.
         * If 'ref' is NULL, then don't exist a reference 'el.att.name' -> pointed.*/
        if (!ref) {
                fprintf (stderr, "'%s' attribute of '%s' element can't point to '%s' element\n",
                         att.name().c_str(), el.elementName().c_str(), pointed -> elementName().c_str());
                msgs.push_back(pair<void *, string> (el.data(),
                                                    messageFactory.createMessage(3004, 3, att.name().c_str(),
                                                                                 el.elementName().c_str(),
                                                                                 pointed -> elementName().c_str())));
                return;
        }


        //////////////////////////////////////////////////////////////////////////////////////////////
        // 								SCOPE TEST													//
        //                                                                                          //
        //	This patch will validate the scope of the reference.									//
        //  One reference may be in 5 different scopes:												//
        //																							//
        //	* ANY: The pointed element can be anywhere in the doc									//
        //	* SAME: The 'pointed.scope' must be equal to 'el.scope' 								//
        //																							//
        //	* $THIS.att: The scope element is defined by the 'att' of 'el'							//
        //	* $PARENT.att: The scope element is defined by the 'att' of 'el.parent'					//
        //	* $GRANDPARENT.att: The scope element is defined by the 'att' of 'el.parent.parent'		//
        //////////////////////////////////////////////////////////////////////////////////////////////

        /* ANY scope elements. The pointed element could be at
         * any position on the doc, so perspectives won't be a problem (hope so! =P).*/
        if (ref -> getPerspective() == "ANY")
                anyScopeReferenceValidation (el, *pointed, att, msgs);

        /* SAME_PERSPECTIVE elements. The pointed element must be at
         * the same perspective of the element who reference him.*/
        else if (ref -> getPerspective() == "SAME") {
                if (el.scope() != pointed -> scope()){
                        fprintf (stderr, "'%s' element pointed by '%s' attribute of '%s' element MUST be at same perspective of him\n",
                                        pointed -> elementName().c_str(), att.name().c_str(), el.elementName().c_str());
                        msgs.push_back(pair<void *, string> (el.data(),
                                                            messageFactory.createMessage(3005, 2, att.name().c_str(),
                                                                                         el.elementName().c_str())));
                        model.addElementWithErrorInLastPass(el.id());
                }
        }

        /* $ELEMENT.ATT elements. The pointed element must be
         * a child (just children?) of '$ELEMENT.ATT'.*/
        else
                specificScopeReferenceValidation(el, *pointed, att, *ref, model, msgs);

        //TODO: Test if 'pointed.att' can be pointed by 'el.att'? Rodrigo only save ids in his map..
}

void parseConnector (ModelElement& element, Model& model, set<string>& roles)
{
    vector <virtualId> children = element.children();
    for (int i = 0; i < children.size(); i ++){
        ModelElement *child = model.element(children[i]);
        if (child)
            if (child->elementName() == "simpleAction" || child->elementName() == "simpleCondition"
                    || child->elementName() == "attributeAssessment"){
                Attribute role = child->attribute("role");
                if (role.value() != "")
                    roles.insert(role.value());
            }
        parseConnector(*child, model, roles);
    }
}


void bindValidation (const ModelElement &bind, ModelElement &connector, Model &model,
                     vector<pair<void *, string> > &msgs, Message &messageFactory)
{
    set<string> roles;
    parseConnector(connector, model, roles);

    vector <Attribute> atts = bind.attributes();

    for (int i = 0; i < atts.size(); i++){
        Attribute attribute = atts[i];
        if (attribute.name() == "role"){
            if (!roles.count(attribute.value())){
                msgs.push_back(pair <void *, string> (bind.data(),
                                                             messageFactory.createMessage(3901, 2,
                                                                                          attribute.value().c_str(),
                                                                                          connector.attribute("id").value().
                                                                                          c_str())));
            }
        }
    }

}

//void linkValidation (ModelElement& link, Model& model, )


/* Perform a semantic validation.*/
void SemanticValidation::semanticValidation(const ModelElement &el, Model &model, vector<pair<void *, string> > &msgs, Message &messageFactory)
{
    // TODO: Casos especiais
    if (el.elementName() == "bind") {
        ModelElement * link = model.element(el.parent());
        if (link){
            Attribute xconnector = link->attribute("xconnector");
            if (xconnector.value() != ""){
                vector <ModelElement *> connectors = model.elementByIdentifier(xconnector.value());
                if (connectors.size() > 0){
                    bindValidation(el, *connectors[0], model, msgs, messageFactory);
                }
            }
        }
    }


    vector<Attribute> atts = el.attributes();

    vector<Attribute>::iterator attIt = atts.begin ();
    for (Attribute att; attIt != atts.end (); ++attIt) {
        att = *attIt;

        // These attributes will be validated later
        if (att.name() == "role" || att.name() == "name")
            continue;

        // Att make reference?
        if (Langstruct::isAttributeReferenceDependent(el.elementName(), att.name()))
            referenceValidation(el, att, model, msgs, messageFactory);
    }


}


}
