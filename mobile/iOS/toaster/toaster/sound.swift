//
//  sound.swift
//  toaster
//
//  Created by Jesse Liang on 7/13/16.
//  Copyright © 2016 Jesse Liang. All rights reserved.
//

import UIKit

class sound: UIViewController {
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction func done(sender: UIBarButtonItem) {
        self.dismissViewControllerAnimated(true, completion: nil)
    }
    
}