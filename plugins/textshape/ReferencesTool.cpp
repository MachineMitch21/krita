/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ReferencesTool.h"
#include "TextShape.h"
#include "dialogs/SimpleTableOfContentsWidget.h"
#include "dialogs/SimpleCitationWidget.h"
#include "dialogs/SimpleFootEndNotesWidget.h"
#include "dialogs/SimpleCaptionsWidget.h"
#include "dialogs/TableOfContentsConfigure.h"
#include "dialogs/NotesConfigurationDialog.h"
#include "dialogs/CitationInsertionDialog.h"
//#include "dialogs/InsertBibliographyDialog.h"

#include <KoTextLayoutRootArea.h>
#include <KoCanvasBase.h>
#include <KoTextEditor.h>
#include <KoParagraphStyle.h>
#include <KoTableOfContentsGeneratorInfo.h>
#include <KoBookmark.h>
#include <KoInlineNote.h>
#include <KoTextDocumentLayout.h>
#include <KoInlineTextObjectManager.h>

#include <kdebug.h>

#include <KLocale>
#include <KAction>
#include <QTextDocument>

#include <QMenu>

ReferencesTool::ReferencesTool(KoCanvasBase* canvas): TextTool(canvas),
    m_configure(0),
    m_stocw(0)
{
    createActions();
}

ReferencesTool::~ReferencesTool()
{
}

void ReferencesTool::createActions()
{
    KAction *action = new KAction(i18n("Insert"), this);
    addAction("insert_tableofcentents", action);
    action->setToolTip(i18n("Insert a Table of Contents into the document."));
    connect(action, SIGNAL(triggered()), this, SLOT(insertTableOfContents()));

    action = new KAction(i18n("Configure..."), this);
    addAction("format_tableofcentents", action);
    action->setToolTip(i18n("Configure the Table of Contents"));
    connect(action, SIGNAL(triggered()), this, SLOT(formatTableOfContents()));

    action = new KAction(i18n("Footnote"),this);
    addAction("insert_footnote",action);
    action->setToolTip(i18n("Insert a FootNote into the document."));
    connect(action, SIGNAL(triggered()), this, SLOT(insertFootNote()));

    action = new KAction(i18n("Endnote"),this);
    addAction("insert_endnote",action);
    action->setToolTip(i18n("Insert an EndNote into the document."));
    connect(action, SIGNAL(triggered()), this, SLOT(insertEndNote()));

    action = new KAction(this);
    QIcon *icon = new QIcon("/home/erione/kde4/src/calligra/plugins/textshape/pics/settings-icon1_1.png");
    action->setIcon( *icon );
    addAction("notes_settings",action);
    action->setToolTip(i18n("Footnote/Endnote Settings"));
    connect(action, SIGNAL(triggered()), this, SLOT(openSettings()));

    action = new KAction(i18n("Insert"),this);
    addAction("insert_citation",action);
    action->setToolTip(i18n("Insert a citation into the document."));
    connect(action, SIGNAL(triggered()), this, SLOT(insertCitation()));

    action = new KAction(i18n("Insert"),this);
    addAction("insert_bibliography",action);
    action->setToolTip(i18n("Insert a bibliography into the document."));
    connect(action, SIGNAL(triggered()), this, SLOT(insertBibliography()));
}

void ReferencesTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    TextTool::activate(toolActivation, shapes);
}

void ReferencesTool::deactivate()
{
    TextTool::deactivate();
    canvas()->canvasWidget()->setFocus();
}

QList<QWidget*> ReferencesTool::createOptionWidgets()
{
    QList<QWidget *> widgets;
    m_stocw = new SimpleTableOfContentsWidget(this, 0);
    //SimpleCitationWidget *scw = new SimpleCitationWidget(0);
    m_sfenw = new SimpleFootEndNotesWidget(this,0);
    //SimpleCaptionsWidget *scapw = new SimpleCaptionsWidget(0);
    SimpleCitationWidget *scw = new SimpleCitationWidget(this,0);
    // Connect to/with simple table of contents option widget
    connect(m_stocw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    // Connect to/with simple citation index option widget
    //connect(scw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    // Connect to/with simple citation index option widget
    connect(m_sfenw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    m_stocw->setWindowTitle(i18n("Table of Contents"));
    widgets.append(m_stocw);

    m_sfenw->setWindowTitle(i18n("Footnotes & Endnotes"));
    widgets.append(m_sfenw);
    scw->setWindowTitle(i18n("Citations and Bibliography"));
    widgets.append(scw);
    //widgets.insert(i18n("Citations"), scw);
    //widgets.insert(i18n("Captions"), scapw);
    return widgets;
}

void ReferencesTool::insertTableOfContents()
{
    textEditor()->insertTableOfContents();
}

void ReferencesTool::insertCitation()
{
    CitationInsertionDialog *dialog = new CitationInsertionDialog(textEditor(),canvas()->canvasWidget());
    dialog->show();
}

void ReferencesTool::insertBibliography()
{
    //InsertBibliographyDialog *dialog = new InsertBibliographyDialog(textEditor(), canvas()->canvasWidget());
    //dialog->show();
}

void ReferencesTool::formatTableOfContents()
{
    //if(!m_configure)
   // {
    qDebug()<<"format";
    QTextDocument *m_document = textEditor()->document();
    QMenu *tocList = new QMenu(m_stocw);
    int i = 0;
    QTextBlock firstToCTextBlock;
    for (QTextBlock it = m_document->begin(); it != m_document->end(); it = it.next())
    {
        if (it.blockFormat().hasProperty(KoParagraphStyle::TableOfContentsDocument)) {
            KoTableOfContentsGeneratorInfo *info = it.blockFormat().property(KoParagraphStyle::TableOfContentsData).value<KoTableOfContentsGeneratorInfo*>();
            if (i == 0) {
                firstToCTextBlock = it;
            }
            QAction *action = new QAction(info->m_indexTitleTemplate.text, tocList);
            action->setData(QVariant::fromValue<QTextBlock>(it));
            tocList->addAction(action);
            i++;
        }
    }

    if (i == 0) {
        //no ToCs in the document
        return;
    } else if (i == 1 && firstToCTextBlock.isValid()) {
        m_configure = new TableOfContentsConfigure(textEditor(), firstToCTextBlock, m_stocw);
    } else {
        m_stocw->setToCConfigureMenu(tocList);
        connect(m_stocw->ToCConfigureMenu(), SIGNAL(triggered(QAction *)), SLOT(showConfigureDialog(QAction*)));
        m_stocw->showMenu();
    }
}

void ReferencesTool::showConfigureDialog(QAction *action)
{
    m_configure = new TableOfContentsConfigure(textEditor(), action->data().value<QTextBlock>(), m_stocw);
}

void ReferencesTool::insertFootNote()
{
    //connect(textEditor()->document(),SIGNAL(cursorPositionChanged(QTextCursor)),this,SLOT(disableButtons(QTextCursor)));
    m_note = textEditor()->insertFootNote();
    m_note->setAutoNumbering(m_sfenw->widget.autoNumbering->isChecked());
    if (m_note->autoNumbering()) {
        m_note->setLabel(QString().number(m_note->manager()->autoNumberedFootNotes().count()));
    }
    else {
        m_note->setLabel(m_sfenw->widget.characterEdit->text());
    }

    QTextCursor cursor(m_note->textCursor());
    QString s;
    s.append("Foot");
    s.append(m_note->label());
    KoBookmark *bookmark = new KoBookmark(m_note->textFrame()->document());
    bookmark->setName(s);
    bookmark->setType(KoBookmark::SinglePosition);
    m_note->manager()->insertInlineObject(cursor, bookmark);
}

void ReferencesTool::insertEndNote()
{
    KoInlineTextObjectManager *manager = KoTextDocument(textEditor()->document()).inlineTextObjectManager();
    m_note = textEditor()->insertEndNote();
    m_note->setAutoNumbering(m_sfenw->widget.autoNumbering->isChecked());
    if (m_note->autoNumbering()) {
        m_note->setLabel(QString().number(m_note->manager()->autoNumberedEndNotes().count()));
    }
    else {
        m_note->setLabel(m_sfenw->widget.characterEdit->text());
    }

    QTextCursor cursor(m_note->textCursor());
    m_note->paintNotesBody(cursor);
    QTextCharFormat *fmat = new QTextCharFormat();
    cursor.insertText(" ", *fmat);
    //inserts a bookmark at the cursor
    QString s;
    s.append("End");
    s.append(m_note->label());
    KoBookmark *bookmark = new KoBookmark(m_note->textFrame()->document());
    bookmark->setType(KoBookmark::SinglePosition);
    bookmark->setName(s);
    m_note->manager()->insertInlineObject(cursor, bookmark);
}

void ReferencesTool::openSettings()
{
    NotesConfigurationDialog *dialog = new NotesConfigurationDialog(textEditor()->document(),0);
    dialog->exec();
}

void ReferencesTool::disableButtons(QTextCursor cursor)
{
    if(cursor.currentFrame()->format().intProperty(KoText::SubFrameType) == KoText::NoteFrameType) {
        m_sfenw->widget.addFootnote->setEnabled(false);
        m_sfenw->widget.addEndnote->setEnabled(false);
    } else {
        m_sfenw->widget.addFootnote->setEnabled(true);
        m_sfenw->widget.addEndnote->setEnabled(true);
    }
}

#include <ReferencesTool.moc>
