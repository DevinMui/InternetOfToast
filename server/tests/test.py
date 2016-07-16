import requests

payload = {"color": "light", "amount": 4}

r = requests.post("http://192.168.56.1:3000/toast", json=payload, headers={"Content-Type": "application/json", "Accept": "application/json"})
print r.json()