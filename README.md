# testavprofile
Test a file to see if it complies with a particular A/V profile, e.g. to see if would play on a particular device.

<tt>testavprofile</tt> returns different exit status codes, depending on what needs to happen to make the file playable on the chosen device.

<table style="width=80%; border=1">
<tr><th style="width:20px">Exit Code</th> <th>Description</th></tr>
<tr><td>0</td> <td>no changes needed - file will play unmodified</td></tr>
<tr><td>1</td> <td>not a media file</td></tr>
<tr><td>2</td> <td>compatible version already exists in the same directory</td></tr>
<tr><td>3</td> <td>remuxing required - streams are compatible, but container isn't</td></tr>
<tr><td>4</td> <td>needs to be transcoded to be playable</td></tr>
</table>
