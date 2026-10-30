const express = require('express');
const router = express.Router();
const fs = require('fs');
const path = require('path')
const mqtt = require('mqtt');
const { Ollama } = require('ollama');


const ollama = new Ollama({ host: 'http://192.168.4.60:30068' });

const options = {
  clientId: 'OatmealInstructions',
  clean: true,
  connectTimeout: 4000,
};

const client = mqtt.connect('mqtt://192.168.4.60:1883',options);


client.on('connect', () => {
    console.log('Connected to Mosquitto Broker on TrueNAS!');
});


const isLoggedIn = (req, res, next) => {
  if (req.session.user) {
    next(); 
  } else {
      res.status(401);
    res.redirect('/auth/login'); 
  }
};



router.post('/cookfood', isLoggedIn, async (req, res) => {
    const   { message }  = req.body;
    console.log(message)
    
    userFoodRequest = message;
    try {
        console.log(`Asking AI how to cook: ${userFoodRequest}`);
        
        res.status(202);
        
        
const sousChefResponse = await ollama.chat({
    model: 'llama3.2:3b',
    messages: [{ 
        role: 'system', 
        content: `### SYSTEM CONTEXT
You are the Executive Sous Chef of the ISAC (Individual Sustainable Autonomous Cooker). You translate vague human requests into precise culinary instructions. Your primary directive is waste reduction through minimal portioning.

### ADAPTABILITY PROTOCOL
- While Slot 0 is usually "Oats," you must be AGNOSTIC. If the user mentions "Pasta," "Rice," or "Couscous," assume Slot 0 is the substrate, Slot 1 is the topping/flavor, and Slot 2 is the seasoning.
- Standard "Small" portion is 60ml-80ml. "Large" (Max) is 150ml.

### COT EXAMPLES
User: "I want a tiny bit of sweet oatmeal."
Logic: User wants sweetness (Slot 2) and minimal waste. 
Plan: Total volume 70ml. High Oats (Slot 0), Low Sugar (Slot 2), No Raisins. Target: 95°C for 7 mins (420000ms).

User: "Make me some spicy pasta."
Logic: Adapt Slot 0 to Pasta. 
Plan: Total volume 100ml. Slot 0: Pasta, Slot 1: Sauce, Slot 2: Red Pepper. Target: 100°C for 10 mins (600000ms).

### OUTPUT REQUIREMENT
Describe the dish in terms of: Substrate (Slot 0), Add-on 1 (Slot 1), Add-on 2 (Slot 2), Temperature, and Duration.`
    },
    { role: 'user', content: userFoodRequest }],
});
console.log(sousChefResponse);
console.log("extrapolating")
const lineCookResponse = await ollama.chat({
    model: 'llama3.2:3b',
    messages: [{ 
        role: 'system', 
        content: `### SYSTEM ROLE
You are the "Precision Line Cook." You receive culinary plans and must compile them into EXACT hardware pulses for an ESP32. 

### HARDWARE MATH (STRICT)
- UNIT FORMULA: 1ml = 64.5 pulses (Therefore, 2000 pulses = ~31ml).
- MAX CAPACITY: 10000 total pulses (150ml) across all slots.
- SLOT 0: Base substrate.
- SLOT 1: Add-on A.
- SLOT 2: Add-on B.
- TEMP LIMIT: 30°C to 100°C.

### FEW-SHOT CALIBRATION
Plan: "Small oatmeal (80ml), bit of sugar, hot."
Calculation: 80ml total. Slot 0 (Oats) = 4500 pulses (~70ml). Slot 2 (Sugar) = 600 pulses (~9ml). 
JSON: {"Slot0": 4500, "Slot1": 0, "Slot2": 600, "temp": 95, "time": 420000, "thought": "Calculated 80ml total volume using 64.5 pulses/ml ratio."}

### OUTPUT REQUIREMENT
You MUST return a JSON object ONLY. Precision is vital for hardware safety.`
    },
    { role: 'user', content: `Compile this plan: ${sousChefResponse}` }],
    format: 'json',
});


const instructions = JSON.parse(lineCookResponse.message.content);
console.log('Sending to ESP32:', instructions);
       delete instructions.thought
        
        
        
        client.publish("oatmeal/commands", JSON.stringify(instructions), { qos: 1, retain: false });
        
        
        res.redirect('https://freshbeets.dev/ISAC/');
        res.status(200);
    } catch (error) { 
        console.error('Error processing request:', error);
        res.status(500).send("Failed to create post.");
    }
    
});


module.exports = router;
