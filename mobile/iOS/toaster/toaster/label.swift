//
//  label.swift
//  toaster
//
//  Created by Jesse Liang on 7/13/16.
//  Copyright © 2016 Jesse Liang. All rights reserved.
//

import UIKit

class label: UIViewController {
    
    @IBOutlet weak var text: UITextField!
    
    let global = NSUserDefaults.standardUserDefaults().stringForKey("label")
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a n
        text.text = global
        text.becomeFirstResponder()
        text.clearButtonMode = .Always
    }
    
    func textFieldShouldReturn(textField: UITextField!) -> Bool {
        
        textField.resignFirstResponder()
        
        self.dismissViewControllerAnimated(true, completion: nil)
        
        let local: NSString = text.text!
        
        NSUserDefaults.standardUserDefaults().setObject(local, forKey: "label")
        
        return true
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction func done(sender: UIBarButtonItem) {
        
        text.resignFirstResponder()
        
        self.dismissViewControllerAnimated(true, completion: nil)
        
        let local: NSString = text.text!
        
        NSUserDefaults.standardUserDefaults().setObject(local, forKey: "label")
    }
    
}