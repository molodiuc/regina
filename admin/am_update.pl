#!/usr/bin/perl -w
#
# Regina - A Normal Surface Theory Calculator
# CVS Utility Script
#
# Copyright (c) 2002, Ben Burton
# For further details contact Ben Burton (benb@acm.org).
#
# Updates all Makefile.am lines of the form:
#   ...HEADERS =...
#   ...SOURCES =...
#   ...java_sources =...
#   ...java_classes =...
# within the given source subtrees to reflect all available sources in
# the corresponding directories.
#
# Usage: am_update [ <directory-tree> | path/to/single/Makefile.am ] ...
# Running without any arguments will perform this update in the usual
# directories.
#
# This file is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This file is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307, USA.
#

use strict;

# --- Are we in the top-level source directory? ---

if (! -d 'admin') {
    print STDERR "ERROR: This script should be run from within the top-level source directory.\n";
    exit(1);
}

# --- Get the list of files and directory trees upon which to operate. ---

my $nArgs = @ARGV;
my @dirsAndFiles;

if ($nArgs) {
    @dirsAndFiles = @ARGV;
} else {
    @dirsAndFiles = ( 'engine', 'javaui', 'testsuite' );
}

# --- Update each file or directory tree. ---

foreach my $item (@dirsAndFiles) {
    if (-e "$item") {
        if (-l "$item") {
            print STDERR "ERROR: File $item is a symbolic link.\n";
            exit(1);
        } elsif (-d "$item") {
            updateDir($item);
        } elsif ((-f "$item") && ($item =~ /Makefile.am$/)) {
            updateFile($item);
        } else {
            print STDERR "ERROR: File $item is not a Makefile.am.\n";
            exit(1);
        }
    } else {
        print STDERR "ERROR: File or directory $item does not exist.\n";
        exit(1);
    }
}

# Usage: updateDir($dir)
# Pre: $dir is known to be an existing directory.
#
sub updateDir {
    # Get a list of files in this directory.
    my @files = glob($_[0].'/*');
    foreach my $entry (@files) {
        if ($entry =~ /Makefile.am$/) {
            updateFile($entry);
        } elsif (-d "$entry") {
            updateDir($entry);
        }
    }
}

# Usage: updateFile($file)
# Pre: $file is known to be an existing Makefile.am.
#
sub updateFile {
    my $fullAm = $_[0];
    my $amDir;
    if ($fullAm =~ /^(.*)\/Makefile.am$/) {
        $amDir = $1;
    } elsif ($fullAm eq 'Makefile.am' ) {
        $amDir = ".";
    } else {
        print STDERR "ERROR: Badly formed Makefile.am filename ($fullAm).";
        exit(1);
    }

    print "Checking $amDir/Makefile.am ... ";

    my @HEADERS;
    my @SOURCES;
    my @java_sources;
    my @java_classes;

    foreach my $entry (glob($amDir.'/*')) {
        if ($entry =~ /^\Q${amDir}\E\/(.*\.h)$/) {
            push(@HEADERS, $1);
        } elsif ($entry =~ /^\Q${amDir}\E\/(.*\.tcc)$/) {
            push(@HEADERS, $1);
        } elsif ($entry =~ /^\Q${amDir}\E\/(.*\.cpp)$/) {
            push(@SOURCES, $1);
        } elsif ($entry =~ /^\Q${amDir}\E\/(.*)\.java$/) {
            push(@java_sources, $1.'.java');
            push(@java_classes, $1.'.class');
        }
    }

    if (! open(DATA, $fullAm)) {
        print STDERR "ERROR: Could not open $fullAm for reading.";
        exit(1);
    }
    my @lines = <DATA>;
    my @newLines = ();
    my $changed = 0;
    close(DATA);

    my $newLine;
    foreach my $line (@lines) {
        if ($line =~ /^(.*)HEADERS\s*=/) {
            $newLine = "$1HEADERS =".fileSet(@HEADERS)."\n";
        } elsif ($line =~ /^(.*)SOURCES\s*=/) {
            $newLine = "$1SOURCES =".fileSet(@SOURCES)."\n";
        } elsif ($line =~ /^(.*)java_sources\s*=/) {
            $newLine = "$1java_sources =".fileSet(@java_sources)."\n";
        } elsif ($line =~ /^(.*)java_classes\s*=/) {
            $newLine = "$1java_classes =".fileSet(@java_classes)."\n";
        } else {
            $newLine = $line;
        }
        push(@newLines, $newLine);
        if ($newLine ne $line) {
            $changed = 1;
        }
    }

    if ($changed) {
        if (! open(NEWDATA, '>'.$fullAm)) {
            print STDERR "ERROR: Could not open $fullAm for writing.";
            exit(1);
        }
        foreach my $newLine (@newLines) {
            print NEWDATA "$newLine";
        }
        close(NEWDATA);
        print "UPDATED.\n";
    } else {
        print "ok.\n";
    }
}

# Usage: fileSet(@array_of_filenames)
#
sub fileSet {
    my $ans = '';
    foreach my $file (sort(@_)) {
        $ans = $ans.' '.$file;
    }
    return $ans;
}

