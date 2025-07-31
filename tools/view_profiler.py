import pandas as pd
import plotly.express as px
from dash import Dash, dcc, html, Input, Output
import sys

# --- Parse CLI Argument for CSV File ---
if len(sys.argv) < 2:
    print("Usage: python profiler_viewer.py <path/to/profiler.csv>")
    sys.exit(1)

csv_path = sys.argv[1]
df = pd.DataFrame(pd.read_csv(csv_path), columns=["Time", "File", "Function", "Label", "Duration"])

# Convert columns
df["Time"] = pd.to_datetime(df["Time"], format="%H:%M:%S", errors="coerce")
df["Duration"] = pd.to_numeric(df["Duration"], errors="coerce")
df.dropna(inplace=True)

# --- Start Dash app ---
app = Dash(__name__)

app.layout = html.Div([
    html.H2("Profiler Function Timing"),
    html.Label("Source File"),
    dcc.Dropdown(
        id="file-dropdown",
        options=[{"label": f, "value": f} for f in sorted(df["File"].unique())],
        value=sorted(df["File"].unique())[0]
    ),
    html.Br(),
    html.Label("Function"),
    dcc.Dropdown(id="function-dropdown"),
    html.Br(),
    dcc.Graph(id="timing-graph")
])

@app.callback(
    Output("function-dropdown", "options"),
    Output("function-dropdown", "value"),
    Input("file-dropdown", "value")
)
def update_function_dropdown(selected_file):
    filtered = df[df["File"] == selected_file]
    functions = sorted(filtered["Function"].unique())
    return [{"label": f, "value": f} for f in functions], functions[0] if functions else None

@app.callback(
    Output("timing-graph", "figure"),
    Input("file-dropdown", "value"),
    Input("function-dropdown", "value")
)
def update_graph(selected_file, selected_function):
    filtered = df[(df["File"] == selected_file) & (df["Function"] == selected_function)]
    if filtered.empty:
        return {}
    fig = px.line(
        filtered,
        x="Time",
        y="Duration",
        color="Label",  # Group by label for multiple plot lines
        markers=True,
        title=f"Execution Time: {selected_function}"
    )
    fig.update_layout(
        xaxis_title="Timestamp",
        yaxis_title="Duration (ms)",
        legend_title="Label"
    )
    return fig

if __name__ == "__main__":
    app.run(debug=True)