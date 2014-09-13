
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  iOS User Interface                                                    *
 *                                                                        *
 *  Copyright (c) 1999-2013, Ben Burton                                   *
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

#import "DBChooser/DBChooser.h"
#import "MasterViewController.h"
#import "MBProgressHUD.h"
#import "ReginaDocument.h"

// TODO: Allow renaming documents.
// TODO: Update timestamp after a file is saved.
// TODO: Mark SnapPea files visually.

// Action sheet tags;
enum {
    sheetNew,
    sheetDelete
};

static UIColor* darkGoldenrod = [UIColor colorWithRed:(0xB8 / 256.0) green:(0x86 / 256.0) blue:(0x0B / 256.0) alpha:1.0];
static UIColor* snapPeaGreen = [UIColor colorWithRed:0.0 green:0.5 blue:0.0 alpha:1.0];

#pragma mark - Document Spec

static NSDateFormatter* dateFormatter;

@interface DocumentSpec : NSObject

enum {
    DOCSPEC_TYPE_RGA,
    DOCSPEC_TYPE_TRI,
    DOCSPEC_TYPE_UNKNOWN
};

@property (strong, nonatomic) NSURL* url;
@property (strong, nonatomic, readonly) NSString* name;
@property (strong, nonatomic, readonly) NSAttributedString* displayName;
@property (strong, nonatomic, readonly) NSDate* lastModified;
@property (assign, nonatomic, readonly) int type;

- (id)initWithURL:(NSURL*)url;
+ (id)specWithURL:(NSURL*)url;

- (NSString*)lastModifiedText;

- (NSComparisonResult)compare:(DocumentSpec*)rhs;

@end

@implementation DocumentSpec

- (id)initWithURL:(NSURL *)url
{
    self = [super init];
    if (self) {
        _url = url;

        NSString* ext = url.pathExtension;
        if ([ext isEqualToString:@"rga"])
            _type = DOCSPEC_TYPE_RGA;
        else if ([ext isEqualToString:@"tri"])
            _type = DOCSPEC_TYPE_TRI;
        else
            _type = DOCSPEC_TYPE_UNKNOWN;

        // We don't use the NSURLLocalizedNameKey property, since sometimes this
        // strips the extension and sometimes it does not.
        _name = _url.lastPathComponent;
        switch (_type) {
            case DOCSPEC_TYPE_RGA:
                _name = [_name stringByDeletingPathExtension];
                _displayName = [[NSAttributedString alloc] initWithString:_name];
                break;
            case DOCSPEC_TYPE_TRI: {
                _displayName = [[NSAttributedString alloc] initWithString:_name attributes:@{NSForegroundColorAttributeName: snapPeaGreen}];
                break;
            }
            case DOCSPEC_TYPE_UNKNOWN:
                _displayName = [[NSAttributedString alloc] initWithString:_name attributes:@{NSForegroundColorAttributeName: darkGoldenrod}];
                break;
        }

        NSError* err;
        NSDate* date;
        if ([url getResourceValue:&date forKey:NSURLContentModificationDateKey error:&err])
            _lastModified = date;
        else {
            NSLog(@"Error querying file modification time for %@: %@", url, err.localizedDescription);
            _lastModified = nil;
        }
    }
    return self;
}

+ (id)specWithURL:(NSURL *)url
{
    return [[DocumentSpec alloc] initWithURL:url];
}

- (NSString *)lastModifiedText
{
    if (! dateFormatter) {
        dateFormatter = [[NSDateFormatter alloc] init];
        dateFormatter.dateStyle = NSDateFormatterMediumStyle;
        dateFormatter.timeStyle = NSDateFormatterShortStyle;
        dateFormatter.doesRelativeDateFormatting = YES;
    }
    return [dateFormatter stringFromDate:self.lastModified];
}

- (NSComparisonResult)compare:(DocumentSpec *)rhs
{
    return [self.name caseInsensitiveCompare:rhs.name];
}

@end

#pragma mark - Master view controller

@interface MasterViewController () <UIActionSheetDelegate, NSURLSessionDownloadDelegate> {
    NSIndexPath* actionIndexPath;
    MBProgressHUD* dropboxHUD;
    UIView* rootView;
}
@property (strong, nonatomic) NSMutableArray *docURLs;
@end

@implementation MasterViewController

- (void)awakeFromNib
{
    self.preferredContentSize = CGSizeMake(320.0, 600.0);
    [super awakeFromNib];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.navigationItem.leftBarButtonItem = self.editButtonItem;
    
    UIBarButtonItem *addButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAdd target:self action:@selector(newDocument:)];
    self.navigationItem.rightBarButtonItem = addButton;

    self.docURLs = [NSMutableArray array];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    [self refreshDocURLs];
}

- (BOOL)openURL:(NSURL *)url preferredName:(NSURL *)name
{
    if (! url) {
        UIAlertView* alert = [[UIAlertView alloc]
                              initWithTitle:@"Missing URL"
                              message:@"I have been asked to open a data file, but no URL was given."
                              delegate:nil
                              cancelButtonTitle:@"Close"
                              otherButtonTitles:nil];
        [alert show];
        return NO;
    }
    
    if (! [url isFileURL]) {
        if ([url.scheme hasPrefix:@"db-"]) {
            // Looks like a Dropbox URL.
            return [[DBChooser defaultChooser] handleOpenURL:url];
        } else {
            // Download the contents of the URL, and save it to the local documents directory.
            NSURLSessionConfiguration* config = [NSURLSessionConfiguration defaultSessionConfiguration];
            NSURLSession* session = [NSURLSession sessionWithConfiguration:config delegate:self delegateQueue:nil];
            NSURLSessionDownloadTask* task = [session downloadTaskWithURL:url];
            
            rootView = [UIApplication sharedApplication].keyWindow.rootViewController.view;
            dropboxHUD = [MBProgressHUD showHUDAddedTo:rootView animated:YES];
            dropboxHUD.mode = MBProgressHUDModeDeterminateHorizontalBar;
            dropboxHUD.progress = 0.0;
            dropboxHUD.labelText = @"Downloading...";

            [task resume];
            return YES;
        }
    }
    
    // This request may be coming from some other app, and this app may be
    // in the middle of something completely different.
    // Wind back anything that is happening in this app at present.
    // This will (amongst other things) have the effect of closing any document
    // that might currently be open.
    [self.navigationController popToRootViewControllerAnimated:NO];
    
    // Open the given document.
    ReginaDocument* doc = [ReginaDocument documentWithInboxURL:url preferredName:nil];
    if (doc) {
        [self performSegueWithIdentifier:@"openInbox" sender:doc];
        return YES;
    } else {
        UIAlertView* alert = [[UIAlertView alloc]
                              initWithTitle:@"Could Not Save on Device"
                              message:nil
                              delegate:nil
                              cancelButtonTitle:@"Close"
                              otherButtonTitles:nil];
        [alert show];
        return NO;
    }
}

- (BOOL)openURL:(NSURL *)url
{
    return [self openURL:url preferredName:nil];
}

- (void)newDocument:(id)sender
{
    UIActionSheet* sheet = [[UIActionSheet alloc] initWithTitle:nil
                                                       delegate:self
                                              cancelButtonTitle:@"Cancel"
                                         destructiveButtonTitle:nil
                                              otherButtonTitles:@"New document", @"Import from Dropbox", nil];
    sheet.tag = sheetNew;
    [sheet showFromBarButtonItem:self.navigationItem.rightBarButtonItem animated:YES];
}

- (ReginaDocument *)documentForIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section == 0)
        return (indexPath.row == 0 ? [ReginaDocument documentWithExample:[Example intro]] : nil);
    else
        return [ReginaDocument documentWithURL:[self.docURLs[indexPath.row] url]];
}

- (void)refreshDocURLs
{
    [self.docURLs removeAllObjects];
    
    NSArray* contents = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:[ReginaDocument docsDir]
                                                      includingPropertiesForKeys:@[NSURLContentModificationDateKey,
                                                                                   NSURLIsDirectoryKey]
                                                                         options:nil
                                                                           error:nil];

    NSError* err;
    NSNumber* isDir;
    for (NSURL* url in [contents objectEnumerator]) {
        if ([url getResourceValue:&isDir forKey:NSURLIsDirectoryKey error:&err]) {
            if (isDir.boolValue)
                continue;
        } else {
            NSLog(@"Error scanning documents directory: %@", err.localizedDescription);
        }
        [self.docURLs addObject:[DocumentSpec specWithURL:url]];
    }

    /*
    [self.docURLs sortUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"lastModified" ascending:NO],
                                         [NSSortDescriptor sortDescriptorWithKey:@"url" ascending:YES]]];
    */
    [self.docURLs sortUsingSelector:@selector(compare:)];

    [self.tableView reloadData];
}

#pragma mark - URL Session

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didFinishDownloadingToURL:(NSURL *)location
{
    NSLog(@"Downloaded to: %@", location);
 
    [MBProgressHUD hideHUDForView:rootView animated:NO];
    dropboxHUD = nil;
    
    ReginaDocument* doc = [ReginaDocument documentWithInboxURL:location preferredName:downloadTask.currentRequest.URL];
    if (doc)
        [self performSegueWithIdentifier:@"openInbox" sender:doc];
    else {
        UIAlertView* alert = [[UIAlertView alloc]
                              initWithTitle:@"Could Not Save on Device"
                              message:nil
                              delegate:nil
                              cancelButtonTitle:@"Close"
                              otherButtonTitles:nil];
        [alert show];
    }
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didWriteData:(int64_t)bytesWritten totalBytesWritten:(int64_t)totalBytesWritten totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite
{
    if (dropboxHUD && totalBytesExpectedToWrite > 0)
        dropboxHUD.progress = (float(totalBytesWritten)) / (float(totalBytesExpectedToWrite));
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error
{
    if (error) {
        NSLog(@"Download error.");
        [MBProgressHUD hideHUDForView:rootView animated:NO];
        dropboxHUD = nil;
        UIAlertView* alert = [[UIAlertView alloc]
                              initWithTitle:@"Could Not Download"
                              message:nil
                              delegate:nil
                              cancelButtonTitle:@"Close"
                              otherButtonTitles:nil];
        [alert show];
    }
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didResumeAtOffset:(int64_t)fileOffset expectedTotalBytes:(int64_t)expectedTotalBytes
{
    // This should not be called, since we do not provide a way to resume downloads.
    NSLog(@"Unexpected NSURLSessionDownloadDelegate call to didResumeAtOffset.");
}

#pragma mark - Table View

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 2;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    if (section == 0)
        return 2;
    else
        return self.docURLs.count;
}

- (NSString*)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
    if (section == 0)
        return @"Examples";
    else
        return @"Documents";
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell;
    
    if (indexPath.section == 0) {
        if (indexPath.row == 0) {
            cell = [tableView dequeueReusableCellWithIdentifier:@"Example" forIndexPath:indexPath];
            cell.textLabel.text = [Example intro].desc;
        } else {
            cell = [tableView dequeueReusableCellWithIdentifier:@"Census" forIndexPath:indexPath];
        }
    } else {
        DocumentSpec* spec = self.docURLs[indexPath.row];
        cell = [tableView dequeueReusableCellWithIdentifier:@"Document" forIndexPath:indexPath];
        cell.textLabel.attributedText = spec.displayName;
        cell.detailTextLabel.text = spec.lastModifiedText;
    }
    return cell;
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    return (indexPath.section == 1);
}

- (UITableViewCellEditingStyle)tableView:(UITableView *)tableView editingStyleForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return UITableViewCellEditingStyleDelete;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        CGRect cell = [tableView cellForRowAtIndexPath:indexPath].frame;
        
        actionIndexPath = indexPath;
        UIActionSheet* sheet = [[UIActionSheet alloc] initWithTitle:nil delegate:self cancelButtonTitle:@"Cancel" destructiveButtonTitle:@"Delete document" otherButtonTitles:nil];
        sheet.tag = sheetDelete;
        [sheet showFromRect:cell inView:tableView animated:YES];
    }
}

#pragma mark - Action Sheet

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
    switch (actionSheet.tag) {
        case sheetNew:
            if (buttonIndex == 0) {
                // Create a new document.
                [self performSegueWithIdentifier:@"openNew" sender:self];
            } else if (buttonIndex == 1) {
                // Import from Dropbox.
                [[DBChooser defaultChooser] openChooserForLinkType:DBChooserLinkTypeDirect fromViewController:self completion:^(NSArray *results) {
                    if ([results count])
                        [self openURL:[results.firstObject link]];
                }];
            }
            break;
        case sheetDelete:
            if (buttonIndex == actionSheet.destructiveButtonIndex) {
                NSURL* url = [self.docURLs[actionIndexPath.row] url];
                NSLog(@"Deleting document: %@", url);
                if ([[NSFileManager defaultManager] removeItemAtURL:url error:nil]) {
                    [self.docURLs removeObjectAtIndex:actionIndexPath.row];
                    [self.tableView deleteRowsAtIndexPaths:@[actionIndexPath] withRowAnimation:UITableViewRowAnimationFade];
                } else {
                    UIAlertView* alert = [[UIAlertView alloc]
                                          initWithTitle:@"Could Not Delete Document"
                                          message:nil
                                          delegate:nil
                                          cancelButtonTitle:@"Close"
                                          otherButtonTitles:nil];
                    [alert show];
                }
            }
            break;
    }
}

@end
