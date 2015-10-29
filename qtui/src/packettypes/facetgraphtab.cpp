
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  KDE User Interface                                                    *
 *                                                                        *
 *  Copyright (c) 1999-2014, Ben Burton                                   *
 *  For further details contact Ben Burton (bab@debian.org).              *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
 *                                                                        *
 *  As an exception, when this program is distributed through (i) the     *
 *  App Store by Apple Inc.; (ii) the Mac App Store by Apple Inc.; or     *
 *  (iii) Google Play by Google Inc., then that store may impose any      *
 *  digital rights management, device limits and/or redistribution        *
 *  restrictions that are required by its terms of service.               *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful, but   *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *  General Public License for more details.                              *
 *                                                                        *
 *  You should have received a copy of the GNU General Public             *
 *  License along with this program; if not, write to the Free            *
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,       *
 *  MA 02110-1301, USA.                                                   *
 *                                                                        *
 **************************************************************************/

/* end stub */

// Regina core includes:
#include "census/dim2edgepairing.h"
#include "census/nfacepairing.h"
#include "dim2/dim2triangulation.h"
#include "triangulation/ntriangulation.h"

// UI includes:
#include "facetgraphtab.h"
#include "reginaprefset.h"
#include "reginasupport.h"
#include "../messagelayer.h"

#include <fstream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLayout>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QSvgWidget>
#include <QSysInfo>
#include <QStackedWidget>
#include <QTemporaryFile>

#if TODO
#include "graphviz/gvc.h"
#endif

extern gvplugin_library_t gvplugin_neato_layout_LTX_library;
extern gvplugin_library_t gvplugin_core_LTX_library;

FacetGraphTab::FacetGraphTab(FacetGraphData* useData,
        PacketTabbedViewerTab* useParentUI) :
        PacketViewerTab(useParentUI),
        data(useData), neverDrawn(true),
        graphvizExec(ReginaPrefSet::global().triGraphvizExec),
        graphvizLabels(ReginaPrefSet::global().triGraphvizLabels) {
    ui = new QWidget();
    QBoxLayout* baseLayout = new QVBoxLayout(ui);
    stack = new QStackedWidget(ui);

    // Information and error layers.
    layerInfo = new MessageLayer("dialog-information", tr("Initialising..."));
    layerError = new MessageLayer("dialog-warning", tr("Initialising..."));
    stack->addWidget(layerInfo);
    stack->addWidget(layerError);

    // Graph layer.
    layerGraph = new QScrollArea();
    // Don't set transparency: a border and lighter background looks
    // kind of nice here.
    // layerGraph->setStyleSheet("QScrollArea, .QWidget { "
    //                             "background-color:transparent; "
    //                         "}");
    graph = new QSvgWidget(layerGraph);
    layerGraph->setWidget(graph);
    layerGraph->setWhatsThis(data->overview());
    stack->addWidget(layerGraph);

    // Finish off.
    baseLayout->addWidget(stack);

    connect(&ReginaPrefSet::global(), SIGNAL(preferencesChanged()),
        this, SLOT(updatePreferences()));
}

void FacetGraphTab::updatePreferences() {
    QString newGraphvizExec = ReginaPrefSet::global().triGraphvizExec;
    bool newGraphvizLabels = ReginaPrefSet::global().triGraphvizLabels;

    // If the graphviz executable or options have changed, redraw the graph.
    // Otherwise do nothing.
    //
    // Note that if the executable *path* hasn't changed but somebody did a
    // reinstall (i.e., the graphviz *behaviour* has changed), they
    // can always hit refresh anyway.
    if (graphvizExec == newGraphvizExec && graphvizLabels == newGraphvizLabels)
        return;

    graphvizExec = newGraphvizExec;
    graphvizLabels = newGraphvizLabels;

    // If we wanted to be polite, we could queue this refresh till
    // later.  In practice there shouldn't be too many viewers
    // actively open and we shouldn't be changing the graphviz
    // executable too often, so it doesn't really seem worth losing
    // sleep over.

    // Actually, we can be a little polite.  If the face pairing
    // graph hasn't been drawn yet (i.e., nobody has ever selected
    // the graph tab), there's no need to refresh since this will
    // be done anyway when the tab is first shown.
    if (! neverDrawn)
        refresh();
}

regina::NPacket* FacetGraphTab::getPacket() {
    return data->getPacket();
}

QWidget* FacetGraphTab::getInterface() {
    return ui;
}

void FacetGraphTab::refresh() {
#if TODO
    showError(tr("<qt>This copy of Regina was not built with "
        "Graphviz support.  Because of this, it cannot display "
        "graphs.<p>"
        "If you downloaded a pre-built package, please contact "
        "the package maintainer for a Graphviz-enabled build.<p>"
        "If you built Regina yourself, please ensure that you have "
        "the Graphviz libraries installed on your system.</qt>"));
    return;
#else
    neverDrawn = false;

    unsigned long n = data->numberOfSimplices();
    if (n == 0) {
        showInfo(tr("<qt>This triangulation is empty.</qt>"));
        return;
    } else if (n > 500) {
        showInfo(tr("<qt>This triangulation contains over 500 %1.<p>"
            "<p>Regina does not display graphs "
            "for such large triangulations.</qt>")
            .arg(data->simplicesName()));
        return;
    }

    std::string dot = data->dot(graphvizLabels);

    char* svg;
    unsigned svgLen;

    GVC_t* gvc = gvContext();
    gvAddLibrary(gvc, &gvplugin_core_LTX_library);
    gvAddLibrary(gvc, &gvplugin_neato_layout_LTX_library);
    Agraph_t* g = agmemread(dot.c_str());
    gvLayout(gvc, g, "neato");
    gvRenderData(gvc, g, "svg", &svg, &svgLen);
    gvFreeLayout(gvc, g);
    agclose(g);
    gvFreeContext(gvc);

    // graph->load(tmpSvg.fileName()); TODO
    graph->resize(graph->sizeHint());

    stack->setCurrentWidget(layerGraph);
#endif
}

void FacetGraphTab::showInfo(const QString& msg) {
    layerInfo->setText(msg);
    stack->setCurrentWidget(layerInfo);
}

void FacetGraphTab::showError(const QString& msg) {
    layerError->setText(msg);
    stack->setCurrentWidget(layerError);
}

std::string Dim2EdgeGraphData::dot(bool withLabels) {
    regina::Dim2EdgePairing* pairing = new regina::Dim2EdgePairing(*tri_);
    std::string ans = pairing->dot(0 /* prefix */, false /* subgraphs */,
        withLabels);
    delete pairing;
    return ans;
}

unsigned long Dim2EdgeGraphData::numberOfSimplices() {
    return tri_->getNumberOfSimplices();
}

QString Dim2EdgeGraphData::simplicesName() {
    return QObject::tr("triangles");
}

QString Dim2EdgeGraphData::overview() {
    return QObject::tr("<qt>The <i>edge pairing graph</i> "
        "of a triangulation describes which triangle edges are "
        "identified with which.<p>Each node of the graph represents "
        "a triangle, and each arc of the graph represents a pair of "
        "triangle edges that are joined together.</qt>");
}

regina::NPacket* Dim2EdgeGraphData::getPacket() {
    return tri_;
}

std::string Dim3FaceGraphData::dot(bool withLabels) {
    regina::NFacePairing* pairing = new regina::NFacePairing(*tri_);
    std::string ans = pairing->dot(0 /* prefix */, false /* subgraphs */,
        withLabels);
    delete pairing;
    return ans;
}

unsigned long Dim3FaceGraphData::numberOfSimplices() {
    return tri_->getNumberOfSimplices();
}

QString Dim3FaceGraphData::simplicesName() {
    return QObject::tr("tetrahedra");
}

QString Dim3FaceGraphData::overview() {
    return QObject::tr("<qt>The <i>face pairing graph</i> "
        "of a triangulation describes which tetrahedron faces are "
        "identified with which.<p>Each node of the graph represents "
        "a tetrahedron, and each arc of the graph represents a pair of "
        "tetrahedron faces that are joined together.</qt>");
}

regina::NPacket* Dim3FaceGraphData::getPacket() {
    return tri_;
}
