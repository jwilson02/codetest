#!/usr/bin/env python3
"""
Performance Profile Analyzer

Analyzes profiling data exported from the game engine and generates
detailed reports with visualizations.

Usage:
    python profile_analyzer.py <input_file> [options]

Options:
    --format {json,csv}    Input file format (default: auto-detect)
    --output <file>        Output report file (default: report.html)
    --plot                 Generate plots (requires matplotlib)
    --threshold <ms>       Highlight functions over threshold (default: 1.0ms)
"""

import json
import csv
import sys
import argparse
from pathlib import Path
from typing import Dict, List, Any, Optional
from dataclasses import dataclass
from datetime import datetime


@dataclass
class ProfileEntry:
    """Single profile entry"""
    name: str
    avg_time_ms: float
    min_time_ms: float
    max_time_ms: float
    call_count: int
    percent_of_frame: float
    children: List['ProfileEntry'] = None

    def __post_init__(self):
        if self.children is None:
            self.children = []


@dataclass
class FrameStats:
    """Frame statistics"""
    frame_time_ms: float
    fps: float
    draw_calls: int
    triangles: int
    objects_rendered: int
    objects_culled: int


@dataclass
class Bottleneck:
    """Performance bottleneck"""
    system: str
    time_ms: float
    budget_ms: float
    over_budget_percent: float
    is_critical: bool


class ProfileAnalyzer:
    """Analyzes profiling data and generates reports"""

    def __init__(self):
        self.frame_stats: Optional[FrameStats] = None
        self.profile_tree: Optional[ProfileEntry] = None
        self.bottlenecks: List[Bottleneck] = []
        self.frame_history: List[Dict[str, Any]] = []

    def load_json(self, filepath: Path) -> None:
        """Load profiling data from JSON file"""
        with open(filepath, 'r') as f:
            data = json.load(f)

        # Parse frame stats
        if 'frameStats' in data:
            stats = data['frameStats']
            self.frame_stats = FrameStats(
                frame_time_ms=stats.get('frameTimeMS', 0),
                fps=stats.get('fps', 0),
                draw_calls=stats.get('drawCalls', 0),
                triangles=stats.get('trianglesRendered', 0),
                objects_rendered=stats.get('objectsRendered', 0),
                objects_culled=stats.get('objectsCulled', 0)
            )

        # Parse profile tree
        if 'profile' in data:
            profile = data['profile']
            self.profile_tree = ProfileEntry(
                name=profile.get('name', 'Unknown'),
                avg_time_ms=profile.get('avgTimeMS', 0),
                min_time_ms=profile.get('minTimeMS', 0),
                max_time_ms=profile.get('maxTimeMS', 0),
                call_count=profile.get('callCount', 0),
                percent_of_frame=profile.get('percentOfFrame', 0)
            )

        # Parse bottlenecks
        if 'bottlenecks' in data:
            for bn in data['bottlenecks']:
                self.bottlenecks.append(Bottleneck(
                    system=bn.get('system', 'Unknown'),
                    time_ms=bn.get('timeMS', 0),
                    budget_ms=bn.get('budgetMS', 0),
                    over_budget_percent=bn.get('overBudgetPercent', 0),
                    is_critical=bn.get('isCritical', False)
                ))

    def load_csv(self, filepath: Path) -> None:
        """Load frame history from CSV file"""
        with open(filepath, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                self.frame_history.append({
                    'frame': int(row['Frame']),
                    'frame_time_ms': float(row['FrameTimeMS']),
                    'fps': float(row['FPS']),
                    'draw_calls': int(row['DrawCalls']),
                    'triangles': int(row['Triangles']),
                    'objects_rendered': int(row['ObjectsRendered']),
                    'objects_culled': int(row['ObjectsCulled'])
                })

        # Calculate average stats
        if self.frame_history:
            avg_frame_time = sum(f['frame_time_ms'] for f in self.frame_history) / len(self.frame_history)
            avg_fps = sum(f['fps'] for f in self.frame_history) / len(self.frame_history)
            avg_draw_calls = sum(f['draw_calls'] for f in self.frame_history) / len(self.frame_history)
            avg_triangles = sum(f['triangles'] for f in self.frame_history) / len(self.frame_history)
            avg_objects_rendered = sum(f['objects_rendered'] for f in self.frame_history) / len(self.frame_history)
            avg_objects_culled = sum(f['objects_culled'] for f in self.frame_history) / len(self.frame_history)

            self.frame_stats = FrameStats(
                frame_time_ms=avg_frame_time,
                fps=avg_fps,
                draw_calls=int(avg_draw_calls),
                triangles=int(avg_triangles),
                objects_rendered=int(avg_objects_rendered),
                objects_culled=int(avg_objects_culled)
            )

    def analyze_performance(self, threshold_ms: float = 1.0) -> Dict[str, Any]:
        """Analyze performance and return summary"""
        analysis = {
            'status': 'unknown',
            'issues': [],
            'recommendations': []
        }

        if not self.frame_stats:
            return analysis

        # Check frame time
        target_frame_time = 16.67  # 60 FPS
        if self.frame_stats.frame_time_ms > target_frame_time:
            analysis['status'] = 'poor'
            analysis['issues'].append(
                f"Frame time ({self.frame_stats.frame_time_ms:.2f}ms) exceeds 60 FPS target ({target_frame_time:.2f}ms)"
            )
        elif self.frame_stats.frame_time_ms > target_frame_time * 0.9:
            analysis['status'] = 'acceptable'
            analysis['issues'].append("Frame time is close to 60 FPS limit")
        else:
            analysis['status'] = 'good'

        # Check bottlenecks
        critical_bottlenecks = [b for b in self.bottlenecks if b.is_critical]
        if critical_bottlenecks:
            analysis['status'] = 'poor'
            for bn in critical_bottlenecks:
                analysis['issues'].append(
                    f"CRITICAL: {bn.system} is {bn.over_budget_percent:.1f}% over budget"
                )
                analysis['recommendations'].append(f"Optimize {bn.system} system")

        # Check draw calls
        if self.frame_stats.draw_calls > 1000:
            analysis['issues'].append(f"High draw call count: {self.frame_stats.draw_calls}")
            analysis['recommendations'].append("Consider batching draw calls or using instancing")

        # Check culling efficiency
        total_objects = self.frame_stats.objects_rendered + self.frame_stats.objects_culled
        if total_objects > 0:
            culling_ratio = self.frame_stats.objects_culled / total_objects
            if culling_ratio < 0.3:
                analysis['issues'].append(
                    f"Low culling efficiency: {culling_ratio*100:.1f}% of objects culled"
                )
                analysis['recommendations'].append("Improve spatial partitioning or frustum culling")

        return analysis

    def generate_report(self, output_file: Path, threshold_ms: float = 1.0) -> None:
        """Generate HTML report"""
        analysis = self.analyze_performance(threshold_ms)

        html = f"""<!DOCTYPE html>
<html>
<head>
    <title>Performance Profile Report</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            margin: 20px;
            background-color: #f5f5f5;
        }}
        .container {{
            max-width: 1200px;
            margin: 0 auto;
            background-color: white;
            padding: 20px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
        }}
        h1, h2 {{
            color: #333;
        }}
        .status {{
            display: inline-block;
            padding: 5px 15px;
            border-radius: 5px;
            font-weight: bold;
        }}
        .status-good {{
            background-color: #4CAF50;
            color: white;
        }}
        .status-acceptable {{
            background-color: #FFC107;
            color: white;
        }}
        .status-poor {{
            background-color: #F44336;
            color: white;
        }}
        .stats-grid {{
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 15px;
            margin: 20px 0;
        }}
        .stat-card {{
            background-color: #f9f9f9;
            padding: 15px;
            border-radius: 5px;
            border-left: 4px solid #2196F3;
        }}
        .stat-value {{
            font-size: 24px;
            font-weight: bold;
            color: #2196F3;
        }}
        .stat-label {{
            color: #666;
            font-size: 14px;
        }}
        .issue {{
            background-color: #ffebee;
            padding: 10px;
            margin: 10px 0;
            border-left: 4px solid #f44336;
        }}
        .recommendation {{
            background-color: #e3f2fd;
            padding: 10px;
            margin: 10px 0;
            border-left: 4px solid #2196F3;
        }}
        .bottleneck {{
            background-color: #fff3e0;
            padding: 10px;
            margin: 10px 0;
            border-left: 4px solid #ff9800;
        }}
        .bottleneck.critical {{
            background-color: #ffebee;
            border-left-color: #f44336;
        }}
        table {{
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
        }}
        th, td {{
            padding: 10px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }}
        th {{
            background-color: #2196F3;
            color: white;
        }}
        .timestamp {{
            color: #999;
            font-size: 12px;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>Performance Profile Report</h1>
        <p class="timestamp">Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>

        <h2>Overall Status: <span class="status status-{analysis['status']}">{analysis['status'].upper()}</span></h2>
"""

        # Frame stats
        if self.frame_stats:
            html += """
        <h2>Frame Statistics</h2>
        <div class="stats-grid">
            <div class="stat-card">
                <div class="stat-label">Frame Time</div>
                <div class="stat-value">{:.2f} ms</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">FPS</div>
                <div class="stat-value">{:.1f}</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Draw Calls</div>
                <div class="stat-value">{}</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Triangles</div>
                <div class="stat-value">{:,}</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Objects Rendered</div>
                <div class="stat-value">{}</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Objects Culled</div>
                <div class="stat-value">{}</div>
            </div>
        </div>
""".format(
                self.frame_stats.frame_time_ms,
                self.frame_stats.fps,
                self.frame_stats.draw_calls,
                self.frame_stats.triangles,
                self.frame_stats.objects_rendered,
                self.frame_stats.objects_culled
            )

        # Issues
        if analysis['issues']:
            html += "\n        <h2>Issues Detected</h2>\n"
            for issue in analysis['issues']:
                html += f'        <div class="issue">{issue}</div>\n'

        # Recommendations
        if analysis['recommendations']:
            html += "\n        <h2>Recommendations</h2>\n"
            for rec in analysis['recommendations']:
                html += f'        <div class="recommendation">{rec}</div>\n'

        # Bottlenecks
        if self.bottlenecks:
            html += "\n        <h2>Performance Bottlenecks</h2>\n"
            for bn in self.bottlenecks:
                critical_class = ' critical' if bn.is_critical else ''
                html += f"""        <div class="bottleneck{critical_class}">
            <strong>{bn.system}</strong>: {bn.time_ms:.2f}ms
            (budget: {bn.budget_ms:.2f}ms, over by {bn.over_budget_percent:.1f}%)
            {'<strong>CRITICAL</strong>' if bn.is_critical else ''}
        </div>\n"""

        html += """
    </div>
</body>
</html>
"""

        with open(output_file, 'w') as f:
            f.write(html)

        print(f"Report generated: {output_file}")

    def print_summary(self) -> None:
        """Print summary to console"""
        print("\n" + "=" * 60)
        print("PERFORMANCE PROFILE SUMMARY")
        print("=" * 60)

        if self.frame_stats:
            print(f"\nFrame Time: {self.frame_stats.frame_time_ms:.2f} ms")
            print(f"FPS: {self.frame_stats.fps:.1f}")
            print(f"Draw Calls: {self.frame_stats.draw_calls}")
            print(f"Triangles: {self.frame_stats.triangles:,}")
            print(f"Objects Rendered: {self.frame_stats.objects_rendered}")
            print(f"Objects Culled: {self.frame_stats.objects_culled}")

        if self.bottlenecks:
            print("\nBottlenecks:")
            for bn in self.bottlenecks:
                status = "CRITICAL" if bn.is_critical else "WARNING"
                print(f"  [{status}] {bn.system}: {bn.time_ms:.2f}ms "
                      f"(over budget by {bn.over_budget_percent:.1f}%)")

        analysis = self.analyze_performance()
        print(f"\nOverall Status: {analysis['status'].upper()}")

        if analysis['recommendations']:
            print("\nRecommendations:")
            for rec in analysis['recommendations']:
                print(f"  - {rec}")

        print("\n" + "=" * 60 + "\n")


def main():
    parser = argparse.ArgumentParser(
        description='Analyze performance profiling data',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    parser.add_argument('input', type=Path, help='Input file (JSON or CSV)')
    parser.add_argument('--format', choices=['json', 'csv'], help='Input format (auto-detect if not specified)')
    parser.add_argument('--output', type=Path, default=Path('report.html'), help='Output report file')
    parser.add_argument('--threshold', type=float, default=1.0, help='Threshold in milliseconds')
    parser.add_argument('--quiet', action='store_true', help='Suppress console output')

    args = parser.parse_args()

    if not args.input.exists():
        print(f"Error: File not found: {args.input}", file=sys.stderr)
        return 1

    # Auto-detect format
    if not args.format:
        args.format = 'json' if args.input.suffix == '.json' else 'csv'

    analyzer = ProfileAnalyzer()

    try:
        if args.format == 'json':
            analyzer.load_json(args.input)
        else:
            analyzer.load_csv(args.input)

        if not args.quiet:
            analyzer.print_summary()

        analyzer.generate_report(args.output, args.threshold)

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
