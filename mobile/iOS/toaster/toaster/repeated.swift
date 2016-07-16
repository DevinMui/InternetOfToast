//
//  repeated.swift
//  toaster
//
//  Created by Jesse Liang on 7/13/16.
//  Copyright © 2016 Jesse Liang. All rights reserved.
//

import UIKit

class repeated: UIViewController {
    
    @IBOutlet weak var table: UITableView!
    
    let menuArr = ["Every Sunday", "Every Monday", "Every Tuesday", "Every Wednesday", "Every Thursday", "Every Friday", "Every Saturday"]
    
    var checked = [false, false, false, false, false, false, false]
    let days = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"]
    let savedArray = NSUserDefaults.standardUserDefaults().arrayForKey("daysArray")
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        
        checked.removeAll()
        for i in 0..<7 {
            checked.append(savedArray![i] as! Bool)
        }
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction func done(sender: UIBarButtonItem) {
        
        self.dismissViewControllerAnimated(true, completion: nil)
        
        var data = [String]()
        
        for i in 0 ..< 7 {
            if (checked[i] == true) {
                data.append(days[i])
            }
        }
        
        var string = String()
        if (data.count != 0) {
            string = data.joinWithSeparator(" ")
        } else {
            string = "Never"
        }
        
        NSUserDefaults.standardUserDefaults().setObject(checked, forKey: "daysArray")
        NSUserDefaults.standardUserDefaults().setObject(string, forKey: "repeat")
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        var cell = tableView.dequeueReusableCellWithIdentifier("cell") as UITableViewCell!
        
        if (savedArray![indexPath.row] as! NSObject == true) {
            cell.accessoryType = .Checkmark
        } else {
            cell.accessoryType = .None
        }
        
        cell.textLabel?.numberOfLines = 10
        cell.textLabel?.text = menuArr[indexPath.row]
        
        return cell!
    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return 7
    }
    
    
    func tableView(tableView: UITableView, canEditRowAtIndexPath indexPath: NSIndexPath) -> Bool {
        return true
    }
    
    func tableView(tableView: UITableView, didSelectRowAtIndexPath indexPath: NSIndexPath) {
        
        if let cell = tableView.cellForRowAtIndexPath(indexPath) {
            if checked[indexPath.row] == false {
                cell.accessoryType = .Checkmark
                checked.removeAtIndex(indexPath.row)
                checked.insert(true, atIndex: indexPath.row)
            } else {
                cell.accessoryType = .None
                checked.removeAtIndex(indexPath.row)
                checked.insert(false, atIndex: indexPath.row)
            }
        }
        
    }
    
    
}