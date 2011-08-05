/* Copyright (c) 2011 Telemidia/PUC-Rio.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    Telemidia/PUC-Rio - initial API and implementation
 */
#ifndef PARENTNOTFOUND_H
#define PARENTNOTFOUND_H

#include <QString>

#include <exception>
using namespace std;

namespace composer{
    namespace core {
        namespace util {

/*!
 \brief Exception to inform that the entity doesn't have a parent.

*/
class ParentNotFound : public exception
{
    private:
        QString element; /*!< TODO */
        QString parent; /*!< TODO */
        QString id; /*!< TODO */
    public:
        /*!
         \brief Constructor

         \param _element
         \param _parent
         \param _id
        */
        ParentNotFound(QString _element, QString _parent ,QString _id) :
                element(_element), parent(_parent), id(_id) {}

        /*!
          \brief Descriptor
        */
        ~ParentNotFound() throw() {}

        /*!
          \brief return a description of what the exception represents.
        */
        const char* what() const throw() {
            QString ret = "Entity (" + parent + ") parent of "
                          + element + "(" + id + ") could not be found!";
            return ret.toStdString().c_str();
        }
};
}}}
#endif // PARENTNOTFOUND_H
