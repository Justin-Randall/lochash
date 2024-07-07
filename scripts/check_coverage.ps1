param (
    [string]$coverageFile,
    [double]$threshold
)

# Load the coverage XML
[xml]$coverageXml = Get-Content $coverageFile

# Extract the line coverage percentage
$lineRate = $coverageXml.coverage.'line-rate'

# Convert the line rate to a percentage
$coverage = [math]::Round([double]$lineRate * 100, 2)

# Compare coverage percentage with the threshold
if ($coverage -lt $threshold) {
    Write-Output "Coverage ($coverage%) is below the threshold ($threshold%)"
    exit 1
}
else {
    Write-Output "Coverage ($coverage%) meets the threshold ($threshold%)"
    exit 0
}
