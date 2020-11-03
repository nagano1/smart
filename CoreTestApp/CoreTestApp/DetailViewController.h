//
//  DetailViewController.h
//  CoreTestApp
//
//  Created by plynth on 2020/02/04.
//  Copyright © 2020 corelang. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface DetailViewController : UIViewController

@property (strong, nonatomic) NSDate *detailItem;
@property (weak, nonatomic) IBOutlet UILabel *detailDescriptionLabel;

@end

