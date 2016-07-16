//
//  ViewController.swift
//  toaster
//
//  Created by Jesse Liang on 7/13/16.
//  Copyright © 2016 Jesse Liang. All rights reserved.
//

import UIKit

class ViewController: UIViewController {
    
    let store = NSUserDefaults.standardUserDefaults()
    
    var alarmlists = [String]()
    var userList = [Int]()

    override func viewDidLoad() {
        super.viewDidLoad()
        
        // Do any additional setup after loading the view, typically from a nib.
        store.setObject("Never", forKey: "repeat")
        store.setObject("Alarm", forKey: "label")
        store.setObject("Alarm", forKey: "sound")
        store.setObject("2 pieces | Light | 15 min delay", forKey: "toast")
        store.setObject("No", forKey: "snooze")
        store.setBool(false, forKey: "snoozeBool")
        store.setInteger(15, forKey: "delay")
        store.setInteger(2, forKey: "amount")
        store.setInteger(0, forKey: "type")
        store.setObject("Light", forKey: "kind")
        
        if (alarmlists.count == 0) {
            let arr = [0]
            store.setObject(arr, forKey: "userList")
        } else {
            let globallist = store.arrayForKey("userList")
            let alarmData = store.arrayForKey("")
        }
        
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


}

