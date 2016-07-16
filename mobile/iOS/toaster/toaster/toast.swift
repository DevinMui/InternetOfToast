//
//  toast.swift
//  toaster
//
//  Created by Jesse Liang on 7/13/16.
//  Copyright © 2016 Jesse Liang. All rights reserved.
//

import UIKit

class toast: UIViewController {
    
    @IBOutlet weak var delayLabel: UILabel!
    @IBOutlet weak var numberLabel: UILabel!
    @IBOutlet weak var segment: UISegmentedControl!
    @IBOutlet weak var addTime: UIStepper!
    @IBOutlet weak var addNumber: UIStepper!
    
    var localTime = Int()
    var localNumber = Int()
    var toast = Int()
    var type = String()
    
    let store = NSUserDefaults.standardUserDefaults()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        
        delayLabel.text = "Delay after alarm:  \(store.integerForKey("delay")) minutes"
        numberLabel.text = "Pieces of toast:  \(store.integerForKey("amount"))"
        
        localTime = store.integerForKey("delay")
        localNumber = store.integerForKey("amount")
        toast = store.integerForKey("type")
        type = store.stringForKey("kind")!
        
        segment.selectedSegmentIndex = toast
        
        addTime.autorepeat = true
        addTime.value = 3
        addTime.maximumValue = 20
        addNumber.autorepeat = true
        addNumber.value = 2
        addNumber.maximumValue = 10
        addNumber.minimumValue = 1
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated
    }
    
    @IBAction func timeChanged(sender: UIStepper) {
        let timedelay = Int(sender.value) * 5
        delayLabel.text = "Delay after alarm:  \(timedelay) minutes"
        localTime = timedelay
    }
    
    @IBAction func numberChanged(sender: UIStepper) {
        
        numberLabel.text = "Pieces of toast:  \(Int(sender.value))"
        localNumber = Int(sender.value)
    }
    
    @IBAction func segment(sender: UISegmentedControl) {
        
        switch segment.selectedSegmentIndex {
        case 0:
            toast = 0
            type = "Light"
        case 1:
            toast = 1
            type = "Medium"
        case 2:
            toast = 2
            type = "Dark"
        default:
            break;
        }
        
    }
    
    @IBAction func done(sender: UIBarButtonItem) {
        
        self.dismissViewControllerAnimated(true, completion: nil)
        
        let string = "\(localNumber) pieces | \(type) | \(localTime) min delay"
        
        store.setInteger(localTime, forKey: "delay")
        store.setInteger(localNumber, forKey: "amount")
        store.setObject(string, forKey: "toast")
        store.setInteger(toast, forKey: "type")
        store.setObject(type, forKey: "kind")
    }
    
}