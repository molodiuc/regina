
/**************************************************************************
 *                                                                        *
 *  Regina - A normal surface theory calculator                           *
 *  Java user interface                                                   *
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

package normal;

import btools.image.StandardImage;

/**
 * A repository of various images.
 */
public class Images {
    /**
     * A button used to create a new file.
     */
    public static final StandardImage btnFileNew =
        new StandardImage("normal/images/buttons/fileNew.gif", Images.class);
    /**
     * A button representing a Jython console.
     */
    public static final StandardImage btnConsole =
        new StandardImage("normal/images/buttons/console.gif", Images.class);
    /**
     * A button representing a docked pane.
     */
    public static final StandardImage btnDockDocked =
        new StandardImage("normal/images/buttons/dockDocked.gif",
            Images.class);
    /**
     * A button representing an undocked pane.
     */
    public static final StandardImage btnDockUndocked =
        new StandardImage("normal/images/buttons/dockUndocked.gif",
            Images.class);

    /**
     * The program title image.
     */
    public static final StandardImage mainTitle =
        new StandardImage("normal/images/main/title.jpg", Images.class);
    /**
     * The large program icon.
     */
    public static final StandardImage mainLargeIcon =
        new StandardImage("normal/images/main/largeicon.jpg", Images.class);
    /**
     * The medium program icon.
     */
    public static final StandardImage mainMediumIcon =
        new StandardImage("normal/images/main/mediumicon.gif", Images.class);
    /**
     * The small program icon.
     */
    public static final StandardImage mainSmallIcon =
        new StandardImage("normal/images/main/tinyicon.gif", Images.class);
}

