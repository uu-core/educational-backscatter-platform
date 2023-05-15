# Pico-Backscatter: stats
A statistical analysis script evaluates communication system performance based on Jupyter Notebook.

## Evaluation metrics
- **Time**: the total latency that given amount of Data is delivered to the destination.
- **Reliability**: the ratio of data correctly delivered to the destination.
- **Distance Metric**: the communication distance of the system $D_{1}^2D_{2}^2$.

## Repo Organization
- `log.txt` contains log file received with either CC2500 or CC1352
- `functions.py` contains functions used in the analysis script
- `statistics.ipynb` contains the system evaluation script and visualisation script
