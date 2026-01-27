# Add Phase 1 implementation files to Tests project

$vcxproj = Get-Content 'Tests\Tests.vcxproj' -Raw

# Find where CFringeSegment.cpp is included
$searchPattern = '    <ClCompile Include="..\\DigitMode\\CFringeSegment.cpp">[\s\S]*?    </ClCompile>'

$newFiles = @'
    <ClCompile Include="..\DigitMode\CFringeSegment.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="..\DigitMode\InputHandler.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="..\DigitMode\SelectionManager.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="..\DigitMode\HitTester.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="..\DigitMode\CommandDispatcher.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="..\DigitMode\CursorManager.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="..\DigitMode\TooltipGenerator.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
'@

$updated = $vcxproj -replace $searchPattern, $newFiles

Set-Content 'Tests\Tests.vcxproj' -Value $updated -NoNewline

Write-Host "✓ Added Phase 1 implementation files to Tests project" -ForegroundColor Green
