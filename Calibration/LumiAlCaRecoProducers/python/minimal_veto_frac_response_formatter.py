import pandas as pd

# Read CSV
df = pd.read_csv("minimal_veto_frac_response-2024.txt")

# Round to 4 significant digits (not just decimal places)
df = df.applymap(lambda x: f"{x:.4g}" if isinstance(x, float) else x)

# Save back without scientific notation
df.to_csv("minimal_veto_frac_response-2024-out.txt", index=False)