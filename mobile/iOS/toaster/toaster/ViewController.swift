//
//  ViewController.swift
//  toaster
//
//  Created by Jesse Liang on 7/13/16.
//  Copyright © 2016 Jesse Liang. All rights reserved.
//

import UIKit

class ViewController: UIViewController {
    
    @IBOutlet weak var table: UITableView!
    
    let store = NSUserDefaults.standardUserDefaults()
    
    var alarmlists = [String]()
    let randomData = ["time": "7:00AM", "label": "Alarm", "days": "Mon Tue", "toast": "2 pieces | Medium | 15 min delay"]
    var alarmData = [String]()
    var userList = [Int]()

    override func viewDidLoad() {
        super.viewDidLoad()
        
    }
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(true)

        if (alarmlists.count == 0) {
            let arr = [0]
            store.setObject(arr, forKey: "userList")
        } else {
            let globallist = store.arrayForKey("userList")
            
            for x in 1..<globallist!.count {
                let arraynum = globallist![x] as! Int
                
                let data = store.arrayForKey("alarm#\(arraynum)")
                alarmData = data as! [String]
            }
        }
        
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

    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        var cell = tableView.dequeueReusableCellWithIdentifier("cell") as UITableViewCell!
        
        let string = randomData["time"]!
        let thing = randomData["label"]! + " " + randomData["days"]!
        
        cell.textLabel?.numberOfLines = 10
        cell.textLabel?.text = string
        cell.detailTextLabel?.text = thing
        
        return cell!
    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return 1
    }
    
    
    func tableView(tableView: UITableView, canEditRowAtIndexPath indexPath: NSIndexPath) -> Bool {
        return true
    }
    
    func tableView(tableView: UITableView, didSelectRowAtIndexPath indexPath: NSIndexPath) {
        
        
        
    }

}

