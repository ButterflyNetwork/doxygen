/******************************************************************************
 *
 * $Id$
 *
 *
 * Copyright (C) 1997-2002 by Dimitri van Heesch.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby 
 * granted. No representations are made about the suitability of this software 
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 *
 */
#include "linkedtexthandler.h"
#include "debug.h"
#include <doxmlintf.h>

class LT_Text : public ILT_Text
{
  public:
    LT_Text(const QString &text) : m_text(text) {}
    virtual ~LT_Text() {}

    // ILT_Text
    virtual QString text() const { return m_text; }
    virtual Kind kind() const { return Kind_Text; }
  private:
    QString m_text;
};

class LT_Ref : public ILT_Ref
{
  public:
    LT_Ref() {}
    virtual ~LT_Ref() {}
    void setRefId(const QString &refId) { m_refId=refId; }
    void setText(const QString &text) { m_text=text; }
    void setExtId(const QString &extId) { m_extId=extId; }
    void setTargetKind(TargetKind k) { m_targetKind=k; }

    // ILT_Ref
    virtual QString text() const { return m_text; }
    virtual QString id() const { return m_refId; }
    virtual TargetKind targetKind() const { return m_targetKind; }
    virtual QString external() const { return m_extId; }
    virtual Kind kind() const { return Kind_Ref; }
    
  private:
    QString    m_refId;
    QString    m_extId;
    QString    m_text;
    TargetKind m_targetKind;
};

LinkedTextHandler::LinkedTextHandler(IBaseHandler *parent,
                                     QList<ILinkedText> &children
                                    ) 
 : m_parent(parent), m_children(children)
{
  addStartHandler("ref",this,&LinkedTextHandler::startRef);
  addEndHandler("ref",this,&LinkedTextHandler::endRef);
  m_children.setAutoDelete(TRUE);
  m_ref=0;
}

LinkedTextHandler::~LinkedTextHandler()
{
}

void LinkedTextHandler::start(const char *endTag)
{
  addEndHandler(endTag,this,&LinkedTextHandler::end);
  m_parent->setDelegate(this);
  m_curString="";
}

void LinkedTextHandler::end()
{
  if (!m_curString.isEmpty())
  {
    m_children.append(new LT_Text(m_curString));
    debug(2,"LinkedTextHandler: add text `%s'\n",m_curString.data());
    m_curString="";
  }
  m_parent->setDelegate(0);
}

void LinkedTextHandler::startRef(const QXmlAttributes& attrib)
{
  if (!m_curString.isEmpty())
  {
    m_children.append(new LT_Text(m_curString));
    debug(2,"LinkedTextHandler: add text `%s'\n",m_curString.data());
    m_curString="";
  }
  ASSERT(m_ref==0);
  m_ref = new LT_Ref;
  m_ref->setRefId(attrib.value("refid"));
  m_ref->setExtId(attrib.value("external"));
  ASSERT(attrib.value("kindref")=="compound" || attrib.value("kindref")=="member");
  m_ref->setTargetKind(attrib.value("kindref")=="compound" ? ILT_Ref::Compound : ILT_Ref::Member);
}

void LinkedTextHandler::endRef()
{
  m_ref->setText(m_curString);
  m_children.append(m_ref);
  debug(2,"LinkedTextHandler: add ref `%s'\n",m_ref->text().data());
  m_ref=0;
}

QString LinkedTextHandler::toString(const QList<ILinkedText> &list) 
{
  QListIterator<ILinkedText> li(list);
  QString result;
  ILinkedText *lt;
  for (li.toFirst();(lt=li.current());++li)
  {
    switch(lt->kind())
    {
      case ILinkedText::Kind_Text:
        result+=dynamic_cast<ILT_Text*>(lt)->text();
        break;
      case ILinkedText::Kind_Ref:
        result+=dynamic_cast<ILT_Ref *>(lt)->text();
        break;
    }
  }
  return result;
}
