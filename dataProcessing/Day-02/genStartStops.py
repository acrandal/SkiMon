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
    },
    {
        'run': 6,
        'start_epoch': 1618690083.429093,
        'stop_epoch': 1618690572.8177412
    },
    {
        'run': 7,
        'start_epoch': 1618692253.5362763,
        'stop_epoch': 1618692361.191608
    },
    {
        'run': 8,
        'start_epoch': 1618692895.4173899,
        'stop_epoch': 1618693008.9375966
    },
    {
        'run': 9,
        'start_epoch': 1618693856.7586377,
        'stop_epoch': 1618693997.0236797
    },
    {
        'run': 10,
        'start_epoch': 1618694652.1828067,
        'stop_epoch': 1618694883.2250285
    },
    {
        'run': 11,
        'start_epoch': 1618695496.191555,
        'stop_epoch': 1618695765.0929072
    },
    {
        'run': 12,
        'start_epoch': 1618696245.7153535,
        'stop_epoch': 1618696507.9116518
    },
    {
        'run': 13,
        'start_epoch': 1618696973.3359368,
        'stop_epoch': 1618697145.8078928
    },
    {
        'run': 14,
        'start_epoch': 1618697719.4538515,
        'stop_epoch': 1618697930.2623444
    }
]

outline = json.dumps(runs, indent=4)
print(outline)
