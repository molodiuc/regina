
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Java User Interface                                                   *
 *                                                                        *
 *  Copyright (c) 1999-2001, Ben Burton                                   *
 *  For further details contact Ben Burton (benb@acm.org).                *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful, but   *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *  General Public License for more details.                              *
 *                                                                        *
 *  You should have received a copy of the GNU General Public             *
 *  License along with this program; if not, write to the Free            *
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,        *
 *  MA 02111-1307, USA.                                                   *
 *                                                                        *
 **************************************************************************/

/* end stub */

package normal.packetui.surfaces;

import java.awt.*;
import java.awt.event.*;
import java.math.BigInteger;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import normal.Shell;
import normal.engine.packet.*;
import normal.engine.surfaces.*;
import normal.mainui.TopologyPane;
import normal.packetui.*;
import org.gjt.btools.gui.component.*;

/**
 * An interface through which the user can view the individual
 * coordinates of a normal surface set.  The set that is currently being
 * viewed can be changed by calling <tt>updateSet()</tt>.
 *
 * @see #updateSet
 */
public class CoordinateViewer extends DefaultPacketViewer
        implements PacketInfoUI, ItemListener {
    /**
     * The normal surface set we are examining.
     */
    private NSurfaceSet set;

    /**
     * The packet we are viewing or editing.
     */
    private NPacket packet;

    /**
     * The shell representing the entire program.
     */
    private Shell shell;

    /**
     * The topology pane responsible for this interface.
     */
    private TopologyPane topPane;

    /**
     * The table containing the normal surface coordinates.
     */
    private JTable table;

    /**
     * The model for the table containing the normal surface coordinates.
     */
    private CoordTableModel model;

    /**
     * The flavour of coordinate system we are using for viewing.
     */
    private int flavour;

    /**
     * A combo box in which we can choose the flavour of coordinate
     * system to use for viewing.
     */
    private JComboBox flavourBox;

    /**
     * The label identifying the coordinate flavour selection box.
     */
    private JLabel flavourLabel;

    /**
     * The text to display in <i>flavourLabel</i> during normal interaction.
     */
    private final static String flavourLabelText = "Coordinate System:";

    /**
     * Are we currently editing this packet elsewhere within the overall
     * packet interface that contains this one?
     */
    private boolean isEditingElsewhere;

    /**
     * Are we currently in the process of updating the list of available
     * flavours?
     */
    private boolean updatingFlavours;

    /**
     * Create a new interface to display the normal surface coordinates for the
     * given normal surface set.
     *
     * @param set the set to display, or <tt>null</tt> if no set should
     * be initially displayed.
     * @param packet the packet we are actually viewing or editing.
     * @param shell the shell representing the entire program.
     * @param topPane the topology pane responsible for this interface.
     */
    public CoordinateViewer(NSurfaceSet set, NPacket packet,
            Shell shell, TopologyPane topPane) {
        super();
        this.set = set;
        this.packet = packet;
        this.shell = shell;
        this.topPane = topPane;
        this.isEditingElsewhere = false;
        this.updatingFlavours = false;
        init();
    }

    /**
     * Create the interface elements.
     */
    private void init() {
        flavourBox = new JComboBox(new DefaultComboBoxModel());
        flavourBox.setMaximumSize(flavourBox.getPreferredSize());
        flavourLabel = new JLabel(flavourLabelText);

        JButton crush = new JButton("Crush");

        model = new CoordTableModel();
        table = new JTable();
        table.setAutoResizeMode(table.AUTO_RESIZE_OFF);
            // This ensures we get a horizontal scrollbar!

        Box flavourPanel = Box.createHorizontalBox();
        flavourPanel.add(flavourLabel);
        flavourPanel.add(Box.createRigidArea(new Dimension(6,0)));
        flavourPanel.add(flavourBox);
        flavourPanel.add(Box.createHorizontalGlue());
        flavourPanel.add(crush);

        setLayout(new BorderLayout());
        add(new PaddedPane(flavourPanel, 3, 12, 3, 3), BorderLayout.NORTH);
        add(new JScrollPane(table), BorderLayout.CENTER);

        flavourBox.addItemListener(this);
        crush.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                int nSel = table.getSelectedRowCount();
                if (nSel == 0) {
                    shell.error("No normal surface is selected to crush.");
                } else if (nSel > 1) {
                    shell.error(
                        "More than one normal surface is selected to crush.");
                } else {
                    // Add the new crushed triangulation.
                    NPacket crushed =
                        set.getSurface(table.getSelectedRow()).crush();
                    crushed.setPacketLabel(packet.makeUniqueLabel(
                        "Crushed " + set.getTriangulation().getPacketLabel()));
                    packet.insertChildLast(crushed);

                    // Add the corresponding node to the tree structure.
                    topPane.fireSubtreeWasInserted(crushed,
                        CoordinateViewer.this, true);
                }
            }
        });
    }

    public NPacket getPacket() {
        return packet;
    }

    public void reflectPacket() {
        this.isEditingElsewhere = false;
        flavourLabel.setText(flavourLabelText);

        updateFlavours();
        reflectFlavour();
    }

    public void editingElsewhere() {
        this.isEditingElsewhere = true;
        flavourLabel.setText("Editing...");
        flavourBox.setEnabled(false);
        table.setModel(new DefaultTableModel());
        ((DefaultTableModel)table.getModel()).fireTableStructureChanged();
    }

    /**
     * Requests that the coordinate viewer display the given set of
     * surfaces.  If the given set is different from the set
     * already being displayed, the new set will be displayed.
     * If the two sets represent the same object, this routine will
     * force the coordinate viewer
     * to abandon any information it has stored and reread everything
     * from the given set.
     * <p>
     * This routine should be called whenever a new surface set needs to
     * be displayed or whenever the currently displayed surface set is known
     * to have changed.
     *
     * @param set the set of normal surfaces that this coordinate viewer
     * should display, or <tt>null</tt> if no set should be displayed.
     */
    public void updateSet(NSurfaceSet set) {
        this.set = set;

        updateFlavours();
        reflectFlavour();
    }

    /**
     * Updates the list of available flavours of coordinate system
     * according to the currently selected surface set.
     * The current flavour selection will be preserved if possible.
     */
    private void updateFlavours() {
        updatingFlavours = true;

        Object sel = flavourBox.getSelectedItem();
        flavourBox.removeAllItems();

        Vector viewers = Coordinates.allViewers(set);
        if (viewers.size() == 0) {
            flavourBox.addItem("<Unavailable>");

            flavourBox.setEnabled(false);
        } else {
            Enumeration e = viewers.elements();
            while (e.hasMoreElements())
                flavourBox.addItem(e.nextElement());
            if (((DefaultComboBoxModel)flavourBox.getModel()).
                    getIndexOf(sel) >= 0)
                flavourBox.setSelectedItem(sel);
            else if (set != null)
                flavourBox.setSelectedItem(new Coordinates(set.getFlavour()));

            flavourBox.setEnabled(! isEditingElsewhere);
        }

        updatingFlavours = false;
    }

    /**
     * Updates the interface to display the current surface set using
     * the flavour of coordinate system currently selected in the flavour
     * combo box.
     */
    private void reflectFlavour() {
        Object sel = flavourBox.getSelectedItem();
        if (set == null || sel == null || (! (sel instanceof Coordinates))) {
            DefaultTableModel m = new DefaultTableModel();
            table.setModel(m);
            m.fireTableStructureChanged();
            return;
        }

        flavour = ((Coordinates)sel).getFlavour();
        table.setModel(model);
        model.fireTableStructureChanged();

        TableCellRenderer renderer = new FancyCellRenderer();
        table.getColumnModel().getColumn(1).setCellRenderer(renderer);
        if (set.isEmbeddedOnly()) {
            table.getColumnModel().getColumn(2).setCellRenderer(renderer);
            table.getColumnModel().getColumn(3).setCellRenderer(renderer);
        }

        TableColumn col;
        renderer = new FancyColumnHeaderRenderer(table);
        for (int i=0; i<model.getColumnCount(); i++) {
            col = table.getColumnModel().getColumn(i);
            col.setPreferredWidth(70);
            col.setHeaderRenderer(renderer);
            col.setHeaderValue(new FancyData(model.getColumnName(i),
                model.getColumnToolTip(i)));
        }
    }

    /**
     * For internal use only.
     * Called when the selected item in the flavour box is changed.
     */
    public void itemStateChanged(ItemEvent e) {
        if ((! updatingFlavours) && e.getStateChange() == e.SELECTED)
            reflectFlavour();
    }

    /**
     * Provides a table model for viewing the normal surface coordinates.
     */
    private class CoordTableModel extends AbstractTableModel {
        /**
         * The colour to use for green text.
         */
        private Color green = Color.green.darker().darker().darker();
        /**
         * The colour to use for red text.
         */
        private Color red = Color.red.darker().darker();

        /**
         * Returns the number of rows in the table.
         * @return the number of rows.
         */
        public int getRowCount() {
            return (int)set.getNumberOfSurfaces();
        }

        /**
         * Returns the number of columns in the table.
         * @return the number of columns.
         */
        public int getColumnCount() {
            if (set.isEmbeddedOnly())
                return Coordinates.getNumberOfCoordinates(flavour,
                    set.getTriangulation()) + 5;
            else
                return Coordinates.getNumberOfCoordinates(flavour,
                    set.getTriangulation()) + 3;
        }

        /**
         * Returns the requested value in the table.
         * @param row the row of the requested value.
         * @param col the column of the requested value.
         * @return the value in the given row and column.
         */
        public Object getValueAt(int row, int column) {
            NNormalSurface surface = set.getSurface(row);
            if (set.isEmbeddedOnly())
                switch(column) {
                    case 0:
                        if (surface.isCompact())
                            return surface.getEulerCharacteristic();
                        else
                            return "";
                    case 1:
                        if (! surface.isCompact())
                            return "";
                        int intAns = surface.isOrientable();
                        if (intAns == 1)
                            return new FancyData("Orbl", green);
                        else if (intAns == -1)
                            return new FancyData("Non-orbl", red);
                        else
                            return "Unknown";
                    case 2:
                        if (! surface.isCompact())
                            return "";
                        intAns = surface.isTwoSided();
                        if (intAns == 1)
                            return new FancyData("2", green);
                        else if (intAns == -1)
                            return new FancyData("1", red);
                        else
                            return "Unknown";
                    case 3:
                        if (! surface.isCompact())
                            return "Infinite";
                        else if (surface.hasRealBoundary())
                            return new FancyData("Real Bdry", red);
                        else
                            return new FancyData("Closed", green);
                    case 4:
                        if (surface.isVertexLinking())
                            return "Vtx Link";
                        else if (surface.isSplitting())
                            return "Splitting";
                        else
                            return "";
                    default:
                        BigInteger bigAns = Coordinates.getCoordinate(flavour,
                            surface, column - 5);
                        if (bigAns == null)
                            return "Inf";
                        else if (bigAns.signum() == 0)
                            return "";
                        else
                            return bigAns;
                }
            else
                switch(column) {
                    case 0:
                        if (surface.isCompact())
                            return surface.getEulerCharacteristic();
                        else
                            return "";
                    case 1:
                        if (! surface.isCompact())
                            return "Infinite";
                        else if (surface.hasRealBoundary())
                            return new FancyData("Real Bdry", red);
                        else
                            return new FancyData("Closed", green);
                    case 2:
                        if (surface.isVertexLinking())
                            return "Vtx Link";
                        else if (surface.isSplitting())
                            return "Splitting";
                        else
                            return "";
                    default:
                        BigInteger bigAns = Coordinates.getCoordinate(flavour,
                            surface, column - 3);
                        if (bigAns == null)
                            return "Inf";
                        else if (bigAns.signum() == 0)
                            return "";
                        else
                            return bigAns;
                }
        }

        /**
         * Returns the string name of the requested column.
         * @param column the requested column.
         * @return the string name of the requested column.
         */
        public String getColumnName(int column) {
            if (set.isEmbeddedOnly())
                switch(column) {
                    case 0:
                        return "Euler";
                    case 1:
                        return "Orient";
                    case 2:
                        return "Sides";
                    case 3:
                        return "Bdry";
                    case 4:
                        return "Type";
                    default:
                        return Coordinates.getCoordinateAbbr(flavour,
                            set.getTriangulation(), column - 5);
                }
            else
                switch(column) {
                    case 0:
                        return "Euler";
                    case 1:
                        return "Bdry";
                    case 2:
                        return "Type";
                    default:
                        return Coordinates.getCoordinateAbbr(flavour,
                            set.getTriangulation(), column - 3);
                }
        }

        /**
         * Returns the tooltip for the requested column.
         * @param column the requested column.
         * @return the tooltip for the requested column.
         */
        public String getColumnToolTip(int column) {
            if (set.isEmbeddedOnly())
                switch(column) {
                    case 0:
                        return "Euler characteristic";
                    case 1:
                        return "Orientability";
                    case 2:
                        return "Number of sides";
                    case 3:
                        return "Boundary";
                    case 4:
                        return "Other interesting properties";
                    default:
                        return Coordinates.getCoordinateDesc(flavour,
                            set.getTriangulation(), column - 5);
                }
            else
                switch(column) {
                    case 0:
                        return "Euler characteristic";
                    case 1:
                        return "Boundary";
                    case 2:
                        return "Other interesting properties";
                    default:
                        return Coordinates.getCoordinateDesc(flavour,
                            set.getTriangulation(), column - 3);
                }
        }
    }
}
