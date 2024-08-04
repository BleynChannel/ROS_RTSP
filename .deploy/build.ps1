if ($args[0] -eq "--rebuild") { Remove-Item -Force -Recurse -ErrorAction SilentlyContinue -Path $PSScriptRoot\build\}
New-Item -ItemType Directory -Force -Path $PSScriptRoot\build

Copy-Item -Path $PSScriptRoot\docker-compose.yml -Destination $PSScriptRoot\build

docker build -t bleyn/ros-rtsp:latest $PSScriptRoot\..
if (-not (Test-Path -Path $PSScriptRoot\build\ros_rtsp.tar)) {
	docker image save bleyn/ros-rtsp:latest -o $PSScriptRoot\build\ros_rtsp.tar
}

if (-not (Test-Path -Path $PSScriptRoot\build\data)) {
	New-Item -ItemType Directory -Force -Path $PSScriptRoot\build\data
	Copy-Item -Path $PSScriptRoot\..\launch -Destination $PSScriptRoot\build\data\launch -Recurse
}

'docker load --input $PSScriptRoot\ros-rtsp.tar
docker-compose -f $PSScriptRoot\docker-compose.yml up -d' | Out-File -FilePath $PSScriptRoot\build\deploy.ps1