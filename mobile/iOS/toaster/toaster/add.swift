//
//  add.swift
//  toaster
//
//  Created by Jesse Liang on 7/13/16.
//  Copyright © 2016 Jesse Liang. All rights reserved.
//

import UIKit

class add: UIViewController {

    @IBOutlet weak var timePicker: UIDatePicker!
    @IBOutlet weak var table: UITableView!
    @IBOutlet weak var snooze: UISwitch!
    
    let store = NSUserDefaults.standardUserDefaults()
    
    var rep = String()
    var lab = String()
    var sou = String()
    var toa = String()
    var type = String()
    var amount = Int()
    var delay = Int()

    var menuArr = ["Repeat", "Label", "Sound", "Toast"]
    
    let timeFormatter = NSDateFormatter()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        updateStuff()
        
        
    }
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(true)
        
        updateStuff()
    }
    
    func updateStuff() {
        
        rep = store.stringForKey("repeat")! as String
        lab = store.stringForKey("label")! as String
        sou = store.stringForKey("sound")! as String
        toa = store.stringForKey("toast")! as String
        type = store.stringForKey("kind")! as String
        amount = store.integerForKey("amount")
        delay = store.integerForKey("delay")
        
        table.reloadData()
    }
    
    @IBAction func cancel(sender: UIBarButtonItem) {
        self.dismissViewControllerAnimated(true, completion: nil)
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction func save(sender: UIBarButtonItem) {
        
        var sno = Bool()
        
        let userlist = store.arrayForKey("userList")
        let userid = userlist!.last as! Int + 1
        
        timeFormatter.timeStyle = NSDateFormatterStyle.ShortStyle
        let x = timePicker.date
        
        if (snooze.on == true) {
            sno = true
        } else {
            sno = false
        }
        
        let userData = [rep, lab, sou, sno, delay, type, amount, x, userid]
        
        store.setObject(userData, forKey: "alarm#\(userid)")
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        var cell = tableView.dequeueReusableCellWithIdentifier("cell") as UITableViewCell!
        
        let details = [rep, lab, sou, toa]
        
        cell.textLabel?.numberOfLines = 10
        cell.textLabel?.text = menuArr[indexPath.row]
        cell.detailTextLabel?.text = details[indexPath.row]
        
        return cell!
    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return 4
    }
    
    
    func tableView(tableView: UITableView, canEditRowAtIndexPath indexPath: NSIndexPath) -> Bool {
        return true
    }
    
    func tableView(tableView: UITableView, didSelectRowAtIndexPath indexPath: NSIndexPath) {
        self.performSegueWithIdentifier(menuArr[indexPath.row], sender: nil)
    }
    
}