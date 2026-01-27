# Add DrawModeTest.cpp to Tests project

$vcxproj = Get-Content 'Tests\Tests.vcxproj' -Raw

# Find where to insert (after InputHandlerTest.cpp)
$searchPattern = '    <ClCompile Include="DigitModeTests\\InputHandlerTest.cpp">[\s\S]*?    </ClCompile>'

$newTest = @'
    <ClCompile Include="DigitModeTests\InputHandlerTest.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="DigitModeTests\DrawModeTest.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">NotUsing</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">NotUsing</PrecompiledHeader>
    </ClCompile>
'@

$updated = $vcxproj -replace $searchPattern, $newTest

Set-Content 'Tests\Tests.vcxproj' -Value $updated -NoNewline

Write-Host "✓ Added DrawModeTest.cpp to Tests project" -ForegroundColor Green
