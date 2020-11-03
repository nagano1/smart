//
//  MasterViewController.m
//  CoreTestApp
//
//  Created by plynth on 2020/02/04.
//  Copyright © 2020 corelang. All rights reserved.
//

#import "MasterViewController.hh"
#import "DetailViewController.h"

#include "cpptest.hpp"

@interface MasterViewController ()

@property NSMutableArray *objects;
@end

@implementation MasterViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    

    Cpp *cpp = new Cpp();
    cpp->run("fwiojfwe");
    // Do any additional setup after loading the view, typically from a nib.
    self.navigationItem.leftBarButtonItem = self.editButtonItem;

    UIBarButtonItem *addButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAdd target:self action:@selector(insertNewObject:)];
    self.navigationItem.rightBarButtonItem = addButton;
    self.detailViewController = (DetailViewController *)[[self.splitViewController.viewControllers lastObject] topViewController];


    NSString *path = [[NSBundle mainBundle] pathForResource:@"testlist" ofType:@"json"];
    NSFileHandle *fileHandle = [NSFileHandle fileHandleForReadingAtPath:path];
    NSData *databuffer = [fileHandle readDataToEndOfFile];

    //フォルダー内のファイルからテーマを生成
  //  NSData *databuffer = nil;
//    NSString *specJsonPath = [@"jfiowef" stringByAppendingPathComponent:@"spec.json"];

//    NSFileHandle *file = [NSFileHandle fileHandleForReadingAtPath:specJsonPath];
//    databuffer = [file readDataToEndOfFile];
//    [file closeFile];


    NSError *error = nil;
    NSDictionary *jsonResponse
        = [NSJSONSerialization JSONObjectWithData:databuffer
            options:NSJSONReadingAllowFragments
            error:&error];


    /*
     cplusplus_test
     2020-10-30 00:18:48.970495+0900 CoreTestApp[1634:2600454] stringstream_
     2020-10-30 00:18:48.970527+0900 CoreTestApp[1634:2600454] cplusplus_test
     2020-10-30 00:18:48.970561+0900 CoreTestApp[1634:2600454] test1
     2020-10-30 00:18:48.970604+0900 CoreTestApp[1634:2600454] cplusplus_test
     2020-10-30 00:18:48.970630+0900 CoreTestApp[1634:2600454] ArmAssemblyTest2
     2020-10-30 00:18:48.970654+0900 CoreTestApp[1634:2600454] cplusplus_test
     2020-10-30 00:18:48.970680+0900 CoreTestApp[1634:2600454] printFoo
     2020-10-30 00:18:48.970897+0900 CoreTestApp[1634:2600454] cplusplus_test
     2020-10-30 00:18:48.970945+0900 CoreTestApp[1634:2600454] ThreadQueue
     2020-10-30 00:18:48.971115+0900 CoreTestApp[1634:2600454] concept
     2020-10-30 00:18:48.971164+0900 CoreTestApp[1634:2600454] wakeup_test
     2020-10-30 00:18:48.971292+0900 CoreTestApp[1634:2600454] concept
     2020-10-30 00:18:48.971340+0900 CoreTestApp[1634:2600454] add_consume_test_win32
     2020-10-30 00:18:48.971468+0900 CoreTestApp[1634:2600454] concept
     2020-10-30 00:18:48.971516+0900 CoreTestApp[1634:2600454] add_consume_test
     2020-10-30 00:18:48.971639+0900 CoreTestApp[1634:2600454] concept
     2020-10-30 00:18:48.971687+0900 CoreTestApp[1634:2600454] unordered_map_test
     2020-10-30 00:18:48.971794+0900 CoreTestApp[1634:2600454] parser_test
     2020-10-30 00:18:48.971846+0900 CoreTestApp[1634:2600454] JsonParseTest
     2020-10-30 00:18:48.971969+0900 CoreTestApp[1634:2600454] parser_test
     2020-10-30 00:18:48.972019+0900 CoreTestApp[1634:2600454] char_iteration
     2020-10-30 00:18:48.972141+0900 CoreTestApp[1634:2600454] parser_test
     2020-10-30 00:18:48.972192+0900 CoreTestApp[1634:2600454] utf16Length
     2020-10-30 00:18:48.972387+0900 CoreTestApp[1634:2600454] parser_test
     2020-10-30 00:18:48.972553+0900 CoreTestApp[1634:2600454] utf8ToCodePoint
     2020-10-30 00:18:48.972640+0900 CoreTestApp[1634:2600454] parser_test
     2020-10-30 00:18:48.972757+0900 CoreTestApp[1634:2600454] JustScanLetters
     2020-10-30 00:18:48.972807+0900 CoreTestApp[1634:2600454] parser_test
     2020-10-30 00:18:48.972926+0900 CoreTestApp[1634:2600454] ParserStream
     2020-10-30 00:18:48.972979+0900 CoreTestApp[1634:2600454] parser_test
     2020-10-30 00:18:48.973097+0900 CoreTestApp[1634:2600454] parser_benchmark
     2020-10-30 00:18:48.973149+0900 CoreTestApp[1634:2600454] parser_test
     2020-10-30 00:18:48.973270+0900 CoreTestApp[1634:2600454] charBuffer
     2020-10-30 00:18:48.973323+0900 CoreTestApp[1634:2600454] parser_test
     2020-10-30 00:18:48.973468+0900 CoreTestApp[1634:2600454] CodeNode
     2020-10-30 00:18:48.973542+0900 CoreTestApp[1634:2600454] parser_test
     2020-10-30 00:18:48.973660+0900 CoreTestApp[1634:2600454] ErrorNodeTest_class
     2020-10-30 00:18:48.973753+0900 CoreTestApp[1634:2600454] parser_test
     2020-10-30 00:18:48.974045+0900 CoreTestApp[1634:2600454] ErrorNodeTest
     2020-10-30 00:18:48.974210+0900 CoreTestApp[1634:2600454] parser_test
     2020-10-30 00:18:48.974304+0900 CoreTestApp[1634:2600454] IndentTextTest
     2020-10-30 00:18:48.974432+0900 CoreTestApp[1634:2600454] parser_test
     2020-10-30 00:18:48.974559+0900 CoreTestApp[1634:2600454] Tokenizer
     2020-10-30 00:18:48.974621+0900 CoreTestApp[1634:2600454] SpeedTest
     2020-10-30 00:18:48.974778+0900 CoreTestApp[1634:2600454] memory_order_relaxed_is_fast
     2020-10-30 00:18:48.974930+0900 CoreTestApp[1634:2600454] SpeedTest
     2020-10-30 00:18:48.974985+0900 CoreTestApp[1634:2600454] try_catch_is_slow
     2020-10-30 00:18:48.975120+0900 CoreTestApp[1634:2600454] SpeedTest
     2020-10-30 00:18:48.975232+0900 CoreTestApp[1634:2600454] stack_assign_is_fast
     2020-10-30 00:18:48.975382+0900 CoreTestApp[1634:2600454] SpeedTest
     2020-10-30 00:18:48.975448+0900 CoreTestApp[1634:2600454] functor
     
     */
    for (NSDictionary *json in jsonResponse) {
        NSString * str = json[@"testcasename"];
        NSString * testname = json[@"testname"];

        //NSLog(@"%@", [json description]);
        
        //ThreadQueue
        if ([testname isEqualToString:@"add_consume_test"]){ 
        //if ([testname isEqualToString:@"ThreadQueue"]){ 
            NSLog(@"%@", str);
            NSLog(@"%@", testname);

            dispatch_async(dispatch_get_global_queue(
                DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
                    [NSThread sleepForTimeInterval: 1];
                    int testId = 
                        cpp->runTest([str UTF8String], [testname UTF8String]);
                    [self checkTestEnd: testId max:10000];
            });
            break;
        }


    }
}
- (void) checkTestEnd:(int)testId max:(int)max {
    

    if (max < 0) {
        return;
    }

        double delayInSeconds = 0.16f;
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
        dispatch_after(popTime, dispatch_get_main_queue(), ^(void) {

            Cpp *cpp = new Cpp();
            const char* result = cpp->runTestNext(testId) ;
            NSString * result_str = [[NSString alloc] initWithUTF8String: result];

            if ([result_str isEqualToString:@"[__end_of_test_0];"]) {
                //Log.d("aaa jfowe", "finished log = $logText")
                //test.status.postValue(TestEntryStatus.Failed)

                return;
            } else if ([result_str isEqualToString:@"[__end_of_test_1];"]) {
                //Log.d("aaa jfowe", "finished log = $logText")
                //test.status.postValue(TestEntryStatus.Succeeded)
                return;
            } else {
                if (![result_str isEqualToString:@""]) {
                    NSLog(@"%@",  result_str);
                }
                // logText?.let {
                //     if (it.length > 0) {
                //         test.log.postValue(test.log.value + it)

                //         Log.d(
                //             "aaa jfowe",
                //             "log = [$it]"
                //         )
                //     }
                // }

            }
            [self checkTestEnd: testId max:max-1];

        });

}


- (void)viewWillAppear:(BOOL)animated {
    self.clearsSelectionOnViewWillAppear = self.splitViewController.isCollapsed;
    [super viewWillAppear:animated];
}


- (void)insertNewObject:(id)sender {
    if (!self.objects) {
        self.objects = [[NSMutableArray alloc] init];
    }
    [self.objects insertObject:[NSDate date] atIndex:0];
    NSIndexPath *indexPath = [NSIndexPath indexPathForRow:0 inSection:0];
    [self.tableView insertRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
}


#pragma mark - Segues

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([[segue identifier] isEqualToString:@"showDetail"]) {
        NSIndexPath *indexPath = [self.tableView indexPathForSelectedRow];
        NSDate *object = self.objects[indexPath.row];
        DetailViewController *controller = (DetailViewController *)[[segue destinationViewController] topViewController];
        [controller setDetailItem:object];
        controller.navigationItem.leftBarButtonItem = self.splitViewController.displayModeButtonItem;
        controller.navigationItem.leftItemsSupplementBackButton = YES;
    }
}


#pragma mark - Table View

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.objects.count;
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"Cell" forIndexPath:indexPath];

    NSDate *object = self.objects[indexPath.row];
    cell.textLabel.text = [object description];
    return cell;
}


- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}


- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        [self.objects removeObjectAtIndex:indexPath.row];
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
    }
}


@end
