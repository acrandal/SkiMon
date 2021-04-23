import json

runs = [
    { 
        'run': 1,
        'start_epoch': 1618088876.0083039,
        'stop_epoch': 1618089214.6217465
    },

    {
        'run': 2,
        'start_epoch': 1618090248.120983,
        'stop_epoch': 1618090862.6817179
    },

    {
        'run': 3,
        'start_epoch': 1618092024.3044612,
        'stop_epoch':  1618092145.3702776
    },

    {
        'run': 4,
        'start_epoch': 1618092977.34612,
        'stop_epoch': 1618093330.7205763
    },

    {
        'run': 5,
        'start_epoch': 1618095722.9567192,
        'stop_epoch': 1618095931.8407795
    }
]

outline = json.dumps(runs, indent=4)
print(outline)